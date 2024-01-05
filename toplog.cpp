#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>

void monitorDirectory(const std::string& directory) {
    int inotifyFd = inotify_init(); // Initialize inotify
    if (inotifyFd == -1) {
        std::cerr << "Error initializing inotify\n";
        return;
    }

    int wd = inotify_add_watch(inotifyFd, directory.c_str(), IN_MODIFY); // Add watch
    if (wd == -1) {
        std::cerr << "Cannot watch '" << directory << "'\n";
        close(inotifyFd);
        return;
    }

    const size_t bufLen = (10 * (sizeof(struct inotify_event) + NAME_MAX + 1));
    char buffer[bufLen];

    while (true) {
        ssize_t numRead = read(inotifyFd, buffer, bufLen);
        if (numRead == 0) {
            std::cerr << "Read() from inotify fd returned 0!\n";
            break;
        }

        if (numRead == -1) {
            std::cerr << "Read error\n";
            break;
        }

        for (char *p = buffer; p < buffer + numRead; ) {
            struct inotify_event *event = reinterpret_cast<struct inotify_event *>(p);
            if (event->mask & IN_MODIFY && event->len > 0) {
                std::string fileName = event->name;
                if (fileName.find(".log") != std::string::npos) {
                    std::cout << "Log file modified: " << fileName << '\n';
                }
            }
            p += sizeof(struct inotify_event) + event->len;
        }
    }

    close(inotifyFd);
}
