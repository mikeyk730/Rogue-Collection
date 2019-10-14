#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <vector>
#include "args.h"

namespace
{
    struct ArgWrapper
    {
        ArgWrapper(int argc, char** argv)
        {
            for (int i = 0; i < argc; ++i) {
                AddArg(argv[i]);
            }
        }

        ~ArgWrapper()
        {
            for (char* element : args_)
            {
                delete[] element;
            }
        }

        void AddArg(const std::string& key, const std::string& value)
        {
            AddArg(key);
            AddArg(value);
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

        void AddArg(std::string value)
        {
            args_.push_back(CreateNewString(value));
        }

        std::vector<char*> args_;
    };
}

int StartProcess(int (*start)(int argc, char** argv), int argc, char** argv)
{
    Args args(argc, argv);

    if (args.rogomatic || args.rogomatic_server)
    {
        ArgWrapper wrapper(argc, argv);

        int trogue_pipe[2];
        if (_pipe(trogue_pipe, 256, O_BINARY) == -1)
            exit(1);

        //args_.trogue_fd = std::to_string(trogue_pipe[0]);
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

        //args_.frogue_fd = std::to_string(_fileno(frogue_write));
        wrapper.AddArg("--frogue-fd", std::to_string(_fileno(frogue_write)));
        std::string frogue_read_fd = std::to_string(_fileno(frogue_read));
#else
        int frogue_pipe[2];
        if (_pipe(frogue_pipe, 65536, O_BINARY) == -1)
            exit(1);

        //args_.frogue_fd = std::to_string(frogue_pipe[1]);
        wrapper.AddArg("--frogue-fd", std::to_string(frogue_pipe[1]));
        std::string frogue_read_fd = std::to_string(frogue_pipe[0]);
#endif

        int pid = -1;
        if (args.rogomatic)
        {
            if ((pid = _spawnl(
                P_NOWAIT,
                argv[0],
                argv[0],
                "g",
                "--trogue-fd", trogue_write_fd.c_str(),
                "--frogue-fd", frogue_read_fd.c_str(),
                "--seed", (args.seed.empty() ? "\"\"" : args.seed.c_str()),
                "--genes", (args.genes.empty() ? "\"\"" : args.genes.c_str()),
                NULL)) == -1)
            {
                perror("Spawning Roogomatic failed");
                exit(1);
            }
        }

        auto value = start(wrapper.GetArgc(), wrapper.GetArgv());

        /* Wait until Rogomatic is done processing. */
        int termstat;
        _cwait(&termstat, pid, WAIT_CHILD);
        if (termstat & 0x0)
        {
            printf("Rogomatic failed\n");
        }

        _close(trogue_pipe[1]);
        _close(trogue_pipe[0]);
#ifdef ROGOMATIC_PROTOCOL_DEBUGGING
        fclose(frogue_write);
        fclose(frogue_read);
#else
        _close(frogue_pipe[1]);
        _close(frogue_pipe[0]);
#endif

        return value;
    }

    return start(argc, argv);
}
