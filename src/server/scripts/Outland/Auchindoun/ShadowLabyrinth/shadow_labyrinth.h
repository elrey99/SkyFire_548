/*
 * Copyright (C) 2011-2022 Project SkyFire <https://www.projectskyfire.org/>
 * Copyright (C) 2008-2022 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2022 MaNGOS <https://www.getmangos.eu/>
 * Copyright (C) 2006-2014 ScriptDev2 <https://github.com/scriptdev2/scriptdev2/>
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

#ifndef SHADOW_LABYRINTH_H_
#define SHADOW_LABYRINTH_H_

#define SLScriptName "instance_shadow_labyrinth"

uint32 const EncounterCount = 4;

enum DataTypes
{
    // Encounter States/Boss GUIDs
    DATA_AMBASSADOR_HELLMAW             = 0,
    DATA_BLACKHEART_THE_INCITER         = 1,
    DATA_GRANDMASTER_VORPIL             = 2,
    DATA_MURMUR                         = 3,

    // Additional Data
    DATA_FEL_OVERSEER                   = 4
};

enum CreatureIds
{
    NPC_AMBASSADOR_HELLMAW              = 18731,
    NPC_GRANDMASTER_VORPIL              = 18732,
    NPC_FEL_OVERSEER                    = 18796
};

enum GameObjectIds
{
    GO_REFECTORY_DOOR                   = 183296, // door opened when blackheart the inciter dies
    GO_SCREAMING_HALL_DOOR              = 183295  // door opened when grandmaster vorpil dies
};

enum Misc
{
    ACTION_AMBASSADOR_HELLMAW_INTRO     = 1,
    ACTION_AMBASSADOR_HELLMAW_BANISH    = 2,
};

template<class AI>
AI* GetShadowLabyrinthAI(Creature* creature)
{
    return GetInstanceAI<AI>(creature, SLScriptName);
}

#endif // SHADOW_LABYRINTH_H_

