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

#ifndef SKYFIRESERVER_CORPSE_H
#define SKYFIRESERVER_CORPSE_H

#include "Object.h"
#include "DatabaseEnv.h"
#include "GridDefines.h"
#include "LootMgr.h"

enum class CorpseType
{
    CORPSE_BONES             = 0,
    CORPSE_RESURRECTABLE_PVE = 1,
    CORPSE_RESURRECTABLE_PVP = 2,
    MAX_CORPSE_TYPE = 3
};

// Value equal client resurrection dialog show radius.
#define CORPSE_RECLAIM_RADIUS 39

enum CorpseFlags
{
    CORPSE_FLAG_NONE        = 0x00,
    CORPSE_FLAG_BONES       = 0x01,
    CORPSE_FLAG_UNK1        = 0x02,
    CORPSE_FLAG_UNK2        = 0x04,
    CORPSE_FLAG_HIDE_HELM   = 0x08,
    CORPSE_FLAG_HIDE_CLOAK  = 0x10,
    CORPSE_FLAG_LOOTABLE    = 0x20
};

class Corpse : public WorldObject, public GridObject<Corpse>
{
    public:
        explicit Corpse(CorpseType type = CorpseType::CORPSE_BONES);
        ~Corpse();

        void AddToWorld();
        void RemoveFromWorld();

        bool Create(uint32 guidlow, Map* map);
        bool Create(uint32 guidlow, Player* owner);

        void SaveToDB();
        bool LoadCorpseFromDB(uint32 guid, Field* fields);

        void DeleteBonesFromWorld();
        void DeleteFromDB(SQLTransaction& trans);

        uint64 GetOwnerGUID() const { return GetUInt64Value(CORPSE_FIELD_OWNER); }

        time_t const& GetGhostTime() const { return m_time; }
        void ResetGhostTime() { m_time = time(NULL); }
        CorpseType GetType() const { return m_type; }

        GridCoord const& GetGridCoord() const { return _gridCoord; }
        void SetGridCoord(GridCoord const& gridCoord) { _gridCoord = gridCoord; }

        Loot loot;                                          // remove insignia ONLY at BG
        Player* lootRecipient;
        bool lootForBody;
        bool IsExpired(time_t t) const;

    private:
        CorpseType m_type;
        time_t m_time;
        GridCoord _gridCoord;                                    // gride for corpse position for fast search
};
#endif
