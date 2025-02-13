/*
 * Copyright (C) 2011-2022 Project SkyFire <https://www.projectskyfire.org/>
 * Copyright (C) 2008-2022 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2022 MaNGOS <https://www.getmangos.eu/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Errors.h"

#include <ace/Stack_Trace.h>
#include <ace/OS_NS_unistd.h>
#include <cstdlib>

namespace Skyfire {

void Assert(char const* file, int line, char const* function, char const* message)
{
    ACE_Stack_Trace st;
    fprintf(stderr, "\n%s:%i in %s ASSERTION FAILED:\n  %s\n%s\n",
            file, line, function, message, st.c_str());
    *((volatile int*)NULL) = 0;
    exit(1);
}

void Fatal(char const* file, int line, char const* function, char const* message)
{
    fprintf(stderr, "\n%s:%i in %s FATAL ERROR:\n  %s\n",
                   file, line, function, message);
    ACE_OS::sleep(10);
    *((volatile int*)NULL) = 0;
    exit(1);
}

void Error(char const* file, int line, char const* function, char const* message)
{
    fprintf(stderr, "\n%s:%i in %s ERROR:\n  %s\n",
                   file, line, function, message);
    *((volatile int*)NULL) = 0;
    exit(1);
}

void Warning(char const* file, int line, char const* function, char const* message)
{
    fprintf(stderr, "\n%s:%i in %s WARNING:\n  %s\n",
                   file, line, function, message);
}

} // namespace Skyfire
