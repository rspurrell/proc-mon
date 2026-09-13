#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QMenu>
#include <QProcess>
#include <QSettings>
#include <QSystemTrayIcon>
#include <vector> // Required for tracking processes

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        qWarning() << "System tray unavailable.";
        return 1;
    }

    // TODO: replace with the absolute path to a custom .png or .svg
    QSystemTrayIcon tray(QIcon::fromTheme("utilities-system-monitor"));
    QMenu menu;

    QString configPath = QDir::homePath() + "/.config/proc-mon/proc-mon.conf";
    QSettings settings(configPath, QSettings::IniFormat);

    // Track processes to terminate safely on exit
    std::vector<QProcess*> processes;
    for (const QString& group : settings.childGroups())
    {
        settings.beginGroup(group);
        QString cmd = settings.value("Command").toString();
        bool autostart = settings.value("Autostart", false).toBool();
        settings.endGroup();

        QProcess* process = new QProcess(&app);

        // Use bash to handle arguments, spaces, and environment variables.
        // Prepended command with 'exec' so processes receive termination signal directly.
        // Allows processes to gracefully shutdown on exit.
        process->setProgram("bash");
        process->setArguments({"-c", "exec " + cmd});

        QAction* action = menu.addAction(group + " (Stopped)");

        // Toggle Start/Stop on click
        QObject::connect(action, &QAction::triggered, [process]()
        {
            if (process->state() != QProcess::NotRunning)
            {
                process->terminate(); // Sends SIGTERM for graceful shutdown
            }
            else
            {
                process->start();
            }
        });

        // Update the menu text dynamically based on the process state
        QObject::connect(process, &QProcess::stateChanged, [action, group](QProcess::ProcessState state)
        {
            if (state == QProcess::NotRunning)
            {
                action->setText(group + " (Stopped)");
            }
            else if (state == QProcess::Starting)
            {
                action->setText(group + " (Starting...)");
            }
            else
            {
                action->setText(group + " (Running)");
            }
        });

        // Autostart if flagged in .conf
        if (autostart)
        {
            process->start();
        }

        processes.push_back(process);
    }

    menu.addSeparator();
    QAction* quitAction = menu.addAction("Quit Monitoring");

    // Terminate all processes before exiting.
    QObject::connect(quitAction, &QAction::triggered, [&app, processes]()
    {
        for (QProcess* p : processes)
        {
            if (p->state() != QProcess::NotRunning)
            {
                p->terminate(); // Sends SIGTERM for graceful shutdown

                // Wait up to 3 seconds for the process to clean up and exit
                if (!p->waitForFinished(3000))
                {
                    p->kill(); // Sends SIGKILL if the process hangs
                    p->waitForFinished(1000);
                }
            }
        }
        app.quit();
    });

    tray.setContextMenu(&menu);
    tray.show();

    return app.exec();
}