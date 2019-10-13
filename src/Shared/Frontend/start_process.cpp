#include <io.h>
#include <fcntl.h>
#include <process.h>
#include "args.h"

int StartProcess(int (*start)(Args& args), int argc, char** argv)
{
    Args args(argc, argv);

    if (args.rogomatic) //game process
    {
        int trogue_pipe[2];
        if (_pipe(trogue_pipe, 256, O_BINARY) == -1)
            exit(1);

        args.trogue_fd = std::to_string(trogue_pipe[0]);
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

        args.frogue_fd = std::to_string(_fileno(frogue_write));
        std::string frogue_read_fd = std::to_string(_fileno(frogue_read));
#else
        int frogue_pipe[2];
        if (_pipe(frogue_pipe, 65536, O_BINARY) == -1)
            exit(1);

        args.frogue_fd = std::to_string(frogue_pipe[1]);
        std::string frogue_read_fd = std::to_string(frogue_pipe[0]);
#endif

        int pid;
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

        auto value = start(args);

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

    return start(args);
}