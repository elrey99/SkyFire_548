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

/* ScriptData
SDName: boss_postmaster_malown
SD%Complete: 50
SDComment:
SDCategory: Stratholme
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "stratholme.h"

//Spell ID to summon this guy is 24627 "Summon Postmaster Malown"
//He should be spawned along with three other elites once the third postbox has been opened

enum Says
{
    SAY_KILL                    = 0
};

enum Spells
{
    SPELL_WAILINGDEAD           = 7713,
    SPELL_BACKHAND              = 6253,
    SPELL_CURSEOFWEAKNESS       = 8552,
    SPELL_CURSEOFTONGUES        = 12889,
    SPELL_CALLOFTHEGRAVE        = 17831
};

enum Events
{
    EVENT_WAILINGDEAD          = 1,
    EVENT_BACKHAND             = 2,
    EVENT_CURSEOFWEAKNESS      = 3,
    EVENT_CURSEOFTONGUES       = 4,
    EVENT_CALLOFTHEGRAVE       = 5
};

class boss_postmaster_malown : public CreatureScript
{
    public: boss_postmaster_malown() : CreatureScript("boss_postmaster_malown") { }

        struct boss_postmaster_malownAI : public BossAI
        {
            boss_postmaster_malownAI(Creature* creature) : BossAI(creature, TYPE_MALOWN) { }

            void Reset() OVERRIDE { }

            void EnterCombat(Unit* /*who*/) OVERRIDE
            {
                events.ScheduleEvent(EVENT_WAILINGDEAD, 19000);     // lasts 6 sec
                events.ScheduleEvent(EVENT_BACKHAND, 8000);         // 2 sec stun
                events.ScheduleEvent(EVENT_CURSEOFWEAKNESS, 20000); // lasts 2 mins
                events.ScheduleEvent(EVENT_CURSEOFTONGUES, 22000);
                events.ScheduleEvent(EVENT_CALLOFTHEGRAVE, 25000);
            }

            void KilledUnit(Unit* /*victim*/) OVERRIDE
            {
                Talk(SAY_KILL);
            }

            void UpdateAI(uint32 diff) OVERRIDE
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_WAILINGDEAD:
                            if (rand()%100 < 65) //65% chance to cast
                                DoCastVictim(SPELL_WAILINGDEAD, true);
                            events.ScheduleEvent(EVENT_WAILINGDEAD, 19000);
                            break;
                        case EVENT_BACKHAND:
                            if (rand()%100 < 45) //45% chance to cast
                                DoCastVictim(SPELL_BACKHAND, true);
                            events.ScheduleEvent(EVENT_WAILINGDEAD, 8000);
                            break;
                        case EVENT_CURSEOFWEAKNESS:
                            if (rand()%100 < 3) //3% chance to cast
                                DoCastVictim(SPELL_CURSEOFWEAKNESS, true);
                            events.ScheduleEvent(EVENT_WAILINGDEAD, 20000);
                            break;
                        case EVENT_CURSEOFTONGUES:
                            if (rand()%100 < 3) //3% chance to cast
                                DoCastVictim(SPELL_CURSEOFTONGUES, true);
                            events.ScheduleEvent(EVENT_WAILINGDEAD, 22000);
                            break;
                        case EVENT_CALLOFTHEGRAVE:
                            if (rand()%100 < 5) //5% chance to cast
                                DoCastVictim(SPELL_CALLOFTHEGRAVE, true);
                            events.ScheduleEvent(EVENT_WAILINGDEAD, 25000);
                            break;
                        default:
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const OVERRIDE
        {
            return new boss_postmaster_malownAI(creature);
        }
};

void AddSC_boss_postmaster_malown()
{
    new boss_postmaster_malown();
}
