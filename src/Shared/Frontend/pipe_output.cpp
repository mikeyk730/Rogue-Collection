#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#define _write write
#endif
#include <sstream>
#include <cstring>
#include "pipe_output.h"
#include "utility.h"

# define ctrl(c) (char)((c)&037)
# define CL_TOK ctrl('L')
# define ESC ctrl('[')

static char ones[5000];

PipeOutput::PipeOutput(int pipe_fd)
    : pipe_fd_(pipe_fd)
{
}

void PipeOutput::SetDimensions(Coord dimensions)
{
    dimensions_ = dimensions;
    memset(ones, 0x01, TotalChars());
}

void PipeOutput::UpdateRegion(uint32_t* info, char* dirty)
{
    WriteRogomaticScreen(info, dirty);
}

void PipeOutput::MoveCursor(Coord pos)
{
    WriteRogomaticPosition(pos);
}

void PipeOutput::SetCursor(bool enable)
{
}

void PipeOutput::PlaySound(const std::string& id)
{
}

void PipeOutput::DisplayMessage(const std::string& message)
{
}

void PipeOutput::WriteRogomaticPosition(Coord pos)
{
    std::ostringstream ss;
    ss << ESC << '[' << (pos.y + 1) << ';' << (pos.x + 1) << 'H';
    auto buf = ss.str();
    _write(pipe_fd_, buf.c_str(), buf.size());
}

void PipeOutput::WriteStandoutStart()
{
    std::ostringstream ss;
    ss << ESC << "[7m";
    auto buf = ss.str();
    _write(pipe_fd_, buf.c_str(), buf.size());
}

void PipeOutput::WriteStandoutEnd()
{
    std::ostringstream ss;
    ss << ESC << "[m";
    auto buf = ss.str();
    _write(pipe_fd_, buf.c_str(), buf.size());
}

bool UseStandout(char ch, bool is_standout, int row)
{
    return is_standout && row > 0 && ch >= 'A' && ch <= 'Z';
}

void PipeOutput::WriteRogomaticScreen(uint32_t* data, char* dirty)
{
    int rows = dimensions_.y;
    int cols = dimensions_.x;

    if (memcmp(dirty, ones, TotalChars()) == 0)
    {
        char buf = CL_TOK;
        _write(pipe_fd_, &buf, 1);
    }

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            if (dirty[r*cols + c])
            {
                WriteRogomaticPosition({ c, r });
                while (c < cols && dirty[r*cols + c])
                {
                    auto ch = GetRawCharFromData(data, r, c, cols);
                    bool is_standout = IsStandout(data, r, c, cols);
                    bool use_standout = UseStandout(ch, is_standout, r);
                    if (use_standout)
                    {
                        WriteStandoutStart();
                    }

                    _write(pipe_fd_, &ch, 1);

                    if (use_standout)
                    {
                        WriteStandoutEnd();
                    }

                    ++c;
                }
            }
        }
    }
}

int PipeOutput::TotalChars() const
{
    return dimensions_.x * dimensions_.y;
}
