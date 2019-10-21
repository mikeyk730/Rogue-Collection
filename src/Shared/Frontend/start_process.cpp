#ifdef WIN32
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#define _close close
#define _fileno fileno
#endif
#include <fcntl.h>
#include <cstring>
#include <vector>
#include "args.h"
#include "environment.h"

namespace
{
    struct ArgBuilder
    {
        ArgBuilder(int argc, char** argv)
        {
            for (int i = 0; i < argc; ++i) {
                AddArg(argv[i]);
            }
        }

        ~ArgBuilder()
        {
            for (char* element : args_)
            {
                delete[] element;
            }
        }

        void AddArg(std::string value)
        {
            args_.push_back(CreateNewString(value));
        }

        void AddArg(const std::string& key, const std::string& value)
        {
            AddArg(key);
            AddArg(value);
        }

        void NullTerminate()
        {
            args_.push_back(nullptr);
        }

        int GetArgc() const {
            return args_.size();
        }

        char** GetArgv() {
            return args_.data();
        }

    private:
        char* CreateNewString(const std::string& s)
        {
            size_t size = s.size();
            char* buf = new char[size + 1];
            memcpy(buf, s.c_str(), size + 1);
            return buf;
        }

        std::vector<char*> args_;
    };
}

int StartProcess(int (*start)(int argc, char** argv), int argc, char** argv)
{
    Args args(argc, argv);
    Environment env(args);

    if (args.rogomatic)
    {
        ArgBuilder wrapper(argc, argv);

        int trogue_pipe[2];
#ifdef WIN32
        if (_pipe(trogue_pipe, 256, O_BINARY) == -1)
            exit(1);
#else
        if (pipe(trogue_pipe) == -1)
            exit(1);
#endif

        wrapper.AddArg("--trogue-fd", std::to_string(trogue_pipe[0]));
        std::string trogue_write_fd = std::to_string(trogue_pipe[1]);

#ifdef ROGOMATIC_PROTOCOL_DEBUGGING
        FILE* frogue_write = fopen("ipc.txt", "wb");
        if (!frogue_write) {
            perror("Error opening file for write");
            exit(1);
        }

        FILE* frogue_read = fopen("ipc.txt", "rb");
        if (!frogue_read) {
            perror("Error opening file for read");
            exit(1);
        }

        wrapper.AddArg("--frogue-fd", std::to_string(_fileno(frogue_write)));
        std::string frogue_read_fd = std::to_string(_fileno(frogue_read));
#else
        int frogue_pipe[2];
#ifdef WIN32
        if (_pipe(frogue_pipe, 65536, O_BINARY) == -1)
            exit(1);
#else
        if (pipe(frogue_pipe) == -1)
            exit(1);
#endif

        wrapper.AddArg("--frogue-fd", std::to_string(frogue_pipe[1]));
        std::string frogue_read_fd = std::to_string(frogue_pipe[0]);
#endif

        std::string command(argv[0]);
        env.Get("rogomatic_command", &command);
        std::string profile;
        env.Get("rogomatic_profile", &profile);
        const char* EmptyArg = "\"\"";

        int pid = -1;
#ifdef WIN32
        if (args.rogomatic)
        {
            if ((pid = _spawnl(
                P_NOWAIT,
                command.c_str(),
                command.c_str(),
                "--rogomatic-player",
                "--trogue-fd", trogue_write_fd.c_str(),
                "--frogue-fd", frogue_read_fd.c_str(),
                "--seed", (args.seed.empty() ? EmptyArg : args.seed.c_str()),
                "--genes", (args.genes.empty() ? EmptyArg : args.genes.c_str()),
                "--profile", (profile.empty() ? EmptyArg : profile.c_str()),
                nullptr)) == -1)
            {
                perror("Spawning Roogomatic failed");
                exit(1);
            }
        }
#else
        ArgBuilder spawned_args(0, nullptr);
        spawned_args.AddArg(command);
        spawned_args.AddArg("--rogomatic-player");
        spawned_args.AddArg("--trogue-fd", trogue_write_fd);
        spawned_args.AddArg("--frogue-fd", frogue_read_fd);
        if (!args.seed.empty())
            spawned_args.AddArg("--seed",  args.seed);
        if (!args.genes.empty())
            spawned_args.AddArg("--genes", args.genes);
        if (!profile.empty())
            spawned_args.AddArg("--profile", profile);
        spawned_args.NullTerminate();

        if (args.rogomatic)
        {
            if (posix_spawn(&pid, argv[0], nullptr, nullptr, spawned_args.GetArgv(), environ) != 0)
            {
                perror("Spawning Roogomatic failed");
                exit(1);
            }
        }
#endif

        auto status = start(wrapper.GetArgc(), wrapper.GetArgv());

        /* Wait until Rogomatic is done processing. */
        int termstat;
#ifdef WIN32
        _cwait(&termstat, pid, WAIT_CHILD);
        if (termstat)
        {
            printf("Rogomatic failed\n");
        }
#else
        waitpid(pid, &termstat, WEXITED);
        if (!WIFEXITED(termstat))
        {
            printf("Rogomatic failed\n");
        }

#endif

        _close(trogue_pipe[1]);
        _close(trogue_pipe[0]);
#ifdef ROGOMATIC_PROTOCOL_DEBUGGING
        fclose(frogue_write);
        fclose(frogue_read);
#else
        _close(frogue_pipe[1]);
        _close(frogue_pipe[0]);
#endif

        return status;
    }

    return start(argc, argv);
}
