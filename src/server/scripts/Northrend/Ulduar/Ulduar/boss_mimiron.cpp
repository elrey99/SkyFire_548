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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "ulduar.h"

enum Yells
{
    SAY_AGGRO                                   = 0,
    SAY_HARDMODE_ON                             = 1,
    SAY_MKII_ACTIVATE                           = 2,
    SAY_MKII_SLAY                               = 3,
    SAY_MKII_DEATH                              = 4,
    SAY_VX001_ACTIVATE                          = 5,
    SAY_VX001_SLAY                              = 6,
    SAY_VX001_DEATH                             = 7,
    SAY_AERIAL_ACTIVATE                         = 8,
    SAY_AERIAL_SLAY                             = 9,
    SAY_AERIAL_DEATH                            = 10,
    SAY_V07TRON_ACTIVATE                        = 11,
    SAY_V07TRON_SLAY                            = 12,
    SAY_V07TRON_DEATH                           = 13,
    SAY_BERSERK                                 = 14
};

enum Spells
{
    SPELL_JETPACK                               = 63341,
    SPELL_EMERGENCY_MODE                        = 64582,
    SPELL_SELF_REPAIR                           = 64383,
    SPELL_MAGNETIC_CORE                         = 64444,
    // Leviathan MK II
    SPELL_FLAME_SUPPRESSANT_MK                  = 64570,
    SPELL_NAPALM_SHELL                          = 63666,
    SPELL_PLASMA_BLAST                          = 62977,
    SPELL_PROXIMITY_MINES                       = 63027,
    SPELL_SHOCK_BLAST                           = 63631,
    // VX 001
    SPELL_FLAME_SUPPRESSANT_VX                  = 65192,
    SPELL_FROSTBOMB                             = 64623,
    SPELL_HAND_PULSE                            = 64348,
    SPELL_SPINNING_UP                           = 63414,
    SPELL_RAPID_BURST                           = 63387,
    SPELL_P3WX2_LASER_BARRAGE                   = 63293,
    SPELL_ROCKET_STRIKE                         = 63041,
    SPELL_HEAT_WAVE                             = 63677,
    // Aerial Command Unit
    SPELL_PLASMA_BALL                           = 63689,
    // Additonal spells
    SPELL_MAGNETIC_FIELD                        = 64668,
    SPELL_DEAFENING_SIREN                       = 64616,
    SPELL_WATER_SPRAY                           = 64619,
    SPELL_FROST_BOMB_HARD_MODE                  = 64627,
    SPELL_EXPLOSION                             = 66351,
    SPELL_DISARM                                = 1842,
    SPELL_RIDE_VEHICLE                          = 46598,
    SPELL_TRIGGER_MISSILE                       = 65347,
};

enum Npc
{
    NPC_ASSAULT_BOT                             = 34057,
    NPC_BOMB_BOT                                = 33836,
    NPC_JUNK_BOT                                = 33855,
    NPC_EMERGENCE_FIRE_BOT                      = 34147,
    NPC_FROST_BOMB                              = 34149,
};

class spell_ulduar_proximity_mines : public SpellScriptLoader
{
    public:
        spell_ulduar_proximity_mines() : SpellScriptLoader("spell_ulduar_proximity_mines") { }

        class spell_ulduar_proximity_minesSpellScript : public SpellScript
        {
            PrepareSpellScript(spell_ulduar_proximity_minesSpellScript)

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                for (uint8 i = 0; i < 10; ++i)
                    GetCaster()->CastSpell(GetCaster(), SPELL_TRIGGER_MISSILE, true);
            }

            void Register() OVERRIDE
            {
                OnEffectHitTarget += SpellEffectFn(spell_ulduar_proximity_minesSpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const OVERRIDE
        {
            return new spell_ulduar_proximity_minesSpellScript();
        }
};

void AddSC_boss_mimiron()
{
    new spell_ulduar_proximity_mines();
}
