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

#include "ScriptMgr.h"
#include "OutdoorPvPTF.h"
#include "OutdoorPvPMgr.h"
#include "OutdoorPvP.h"
#include "WorldPacket.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "Language.h"
#include "World.h"

OutdoorPvPTF::OutdoorPvPTF()
{
    m_TypeId = OUTDOOR_PVP_TF;

    m_IsLocked = false;
    m_LockTimer = TF_LOCK_TIME;
    m_LockTimerUpdate = 0;

    m_AllianceTowersControlled = 0;
    m_HordeTowersControlled = 0;

    hours_left = 6;
    second_digit = 0;
    first_digit = 0;
}

OPvPCapturePointTF::OPvPCapturePointTF(OutdoorPvP* pvp, OutdoorPvPTF_TowerType type)
: OPvPCapturePoint(pvp), m_TowerType(type), m_TowerState(TF_TOWERSTATE_N)
{
    SetCapturePointData(TFCapturePoints[type].entry, TFCapturePoints[type].map, TFCapturePoints[type].x, TFCapturePoints[type].y, TFCapturePoints[type].z, TFCapturePoints[type].o, TFCapturePoints[type].rot0, TFCapturePoints[type].rot1, TFCapturePoints[type].rot2, TFCapturePoints[type].rot3);
}

void OPvPCapturePointTF::FillInitialWorldStates(WorldStateBuilder& builder)
{
    builder.AppendState(TFTowerWorldStates[m_TowerType].n, bool(m_TowerState & TF_TOWERSTATE_N));
    builder.AppendState(TFTowerWorldStates[m_TowerType].h, bool(m_TowerState & TF_TOWERSTATE_H));
    builder.AppendState(TFTowerWorldStates[m_TowerType].a, bool(m_TowerState & TF_TOWERSTATE_A));
}

void OutdoorPvPTF::FillInitialWorldStates(WorldStateBuilder& builder)
{
    builder.AppendState(TF_UI_TOWER_SLIDER_POS, 50);
    builder.AppendState(TF_UI_TOWER_SLIDER_N, 100);
    builder.AppendState(TF_UI_TOWER_SLIDER_DISPLAY, 0);

    builder.AppendState(TF_UI_TOWER_COUNT_H, m_HordeTowersControlled);
    builder.AppendState(TF_UI_TOWER_COUNT_A, m_AllianceTowersControlled);
    builder.AppendState(TF_UI_TOWERS_CONTROLLED_DISPLAY, !m_IsLocked);

    builder.AppendState(TF_UI_LOCKED_TIME_MINUTES_FIRST_DIGIT, first_digit);
    builder.AppendState(TF_UI_LOCKED_TIME_MINUTES_SECOND_DIGIT, second_digit);
    builder.AppendState(TF_UI_LOCKED_TIME_HOURS, hours_left);

    builder.AppendState(TF_UI_LOCKED_DISPLAY_NEUTRAL, m_IsLocked && !m_HordeTowersControlled && !m_AllianceTowersControlled);
    builder.AppendState(TF_UI_LOCKED_DISPLAY_HORDE, m_IsLocked && (m_HordeTowersControlled > m_AllianceTowersControlled));
    builder.AppendState(TF_UI_LOCKED_DISPLAY_ALLIANCE, m_IsLocked && (m_HordeTowersControlled < m_AllianceTowersControlled));

    for (OPvPCapturePointMap::iterator itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
    {
        itr->second->FillInitialWorldStates(builder);
    }
}

void OutdoorPvPTF::SendRemoveWorldStates(Player* player)
{
    player->SendUpdateWorldState(TF_UI_TOWER_SLIDER_POS, uint32(0));
    player->SendUpdateWorldState(TF_UI_TOWER_SLIDER_N, uint32(0));
    player->SendUpdateWorldState(TF_UI_TOWER_SLIDER_DISPLAY, uint32(0));

    player->SendUpdateWorldState(TF_UI_TOWER_COUNT_H, uint32(0));
    player->SendUpdateWorldState(TF_UI_TOWER_COUNT_A, uint32(0));
    player->SendUpdateWorldState(TF_UI_TOWERS_CONTROLLED_DISPLAY, uint32(0));

    player->SendUpdateWorldState(TF_UI_LOCKED_TIME_MINUTES_FIRST_DIGIT, uint32(0));
    player->SendUpdateWorldState(TF_UI_LOCKED_TIME_MINUTES_SECOND_DIGIT, uint32(0));
    player->SendUpdateWorldState(TF_UI_LOCKED_TIME_HOURS, uint32(0));

    player->SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_NEUTRAL, uint32(0));
    player->SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_HORDE, uint32(0));
    player->SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_ALLIANCE, uint32(0));

    for (int i = 0; i < TF_TOWER_NUM; ++i)
    {
        player->SendUpdateWorldState(uint32(TFTowerWorldStates[i].n), uint32(0));
        player->SendUpdateWorldState(uint32(TFTowerWorldStates[i].h), uint32(0));
        player->SendUpdateWorldState(uint32(TFTowerWorldStates[i].a), uint32(0));
    }
}

void OPvPCapturePointTF::UpdateTowerState()
{
    m_PvP->SendUpdateWorldState(uint32(TFTowerWorldStates[m_TowerType].n), uint32(bool(m_TowerState & TF_TOWERSTATE_N)));
    m_PvP->SendUpdateWorldState(uint32(TFTowerWorldStates[m_TowerType].h), uint32(bool(m_TowerState & TF_TOWERSTATE_H)));
    m_PvP->SendUpdateWorldState(uint32(TFTowerWorldStates[m_TowerType].a), uint32(bool(m_TowerState & TF_TOWERSTATE_A)));
}

bool OPvPCapturePointTF::HandlePlayerEnter(Player* player)
{
    if (OPvPCapturePoint::HandlePlayerEnter(player))
    {
        player->SendUpdateWorldState(TF_UI_TOWER_SLIDER_DISPLAY, 1);
        uint32 phase = (uint32)ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f);
        player->SendUpdateWorldState(TF_UI_TOWER_SLIDER_POS, phase);
        player->SendUpdateWorldState(TF_UI_TOWER_SLIDER_N, m_neutralValuePct);
        return true;
    }
    return false;
}

void OPvPCapturePointTF::HandlePlayerLeave(Player* player)
{
    player->SendUpdateWorldState(TF_UI_TOWER_SLIDER_DISPLAY, 0);
    OPvPCapturePoint::HandlePlayerLeave(player);
}

bool OutdoorPvPTF::Update(uint32 diff)
{
    bool changed = OutdoorPvP::Update(diff);

    if (changed)
    {
        if (m_AllianceTowersControlled == TF_TOWER_NUM)
        {
            TeamApplyBuff(TEAM_ALLIANCE, TF_CAPTURE_BUFF);
            m_IsLocked = true;
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_NEUTRAL, uint32(0));
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_HORDE, uint32(0));
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_ALLIANCE, uint32(1));
            SendUpdateWorldState(TF_UI_TOWERS_CONTROLLED_DISPLAY, uint32(0));
        }
        else if (m_HordeTowersControlled == TF_TOWER_NUM)
        {
            TeamApplyBuff(TEAM_HORDE, TF_CAPTURE_BUFF);
            m_IsLocked = true;
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_NEUTRAL, uint32(0));
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_HORDE, uint32(1));
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_ALLIANCE, uint32(0));
            SendUpdateWorldState(TF_UI_TOWERS_CONTROLLED_DISPLAY, uint32(0));
        }
        else
        {
            TeamCastSpell(TEAM_ALLIANCE, -TF_CAPTURE_BUFF);
            TeamCastSpell(TEAM_HORDE, -TF_CAPTURE_BUFF);
        }
        SendUpdateWorldState(TF_UI_TOWER_COUNT_A, m_AllianceTowersControlled);
        SendUpdateWorldState(TF_UI_TOWER_COUNT_H, m_HordeTowersControlled);
    }
    if (m_IsLocked)
    {
        // lock timer is down, release lock
        if (m_LockTimer < diff)
        {
            m_LockTimer = TF_LOCK_TIME;
            m_LockTimerUpdate = 0;
            m_IsLocked = false;
            SendUpdateWorldState(TF_UI_TOWERS_CONTROLLED_DISPLAY, uint32(1));
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_NEUTRAL, uint32(0));
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_HORDE, uint32(0));
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_ALLIANCE, uint32(0));
        }
        else
        {
            // worldstateui update timer is down, update ui with new time data
            if (m_LockTimerUpdate < diff)
            {
                m_LockTimerUpdate = TF_LOCK_TIME_UPDATE;
                uint32 minutes_left = m_LockTimer / 60000;
                hours_left = minutes_left / 60;
                minutes_left -= hours_left * 60;
                second_digit = minutes_left % 10;
                first_digit = minutes_left / 10;

                SendUpdateWorldState(TF_UI_LOCKED_TIME_MINUTES_FIRST_DIGIT, first_digit);
                SendUpdateWorldState(TF_UI_LOCKED_TIME_MINUTES_SECOND_DIGIT, second_digit);
                SendUpdateWorldState(TF_UI_LOCKED_TIME_HOURS, hours_left);
            } else m_LockTimerUpdate -= diff;
            m_LockTimer -= diff;
        }
    }
    return changed;
}

void OutdoorPvPTF::HandlePlayerEnterZone(Player* player, uint32 zone)
{
    if (player->GetTeam() == ALLIANCE)
    {
        if (m_AllianceTowersControlled >= TF_TOWER_NUM)
            player->CastSpell(player, TF_CAPTURE_BUFF, true);
    }
    else
    {
        if (m_HordeTowersControlled >= TF_TOWER_NUM)
            player->CastSpell(player, TF_CAPTURE_BUFF, true);
    }
    OutdoorPvP::HandlePlayerEnterZone(player, zone);
}

void OutdoorPvPTF::HandlePlayerLeaveZone(Player* player, uint32 zone)
{
    // remove buffs
    player->RemoveAurasDueToSpell(TF_CAPTURE_BUFF);
    OutdoorPvP::HandlePlayerLeaveZone(player, zone);
}

uint32 OutdoorPvPTF::GetAllianceTowersControlled() const
{
    return m_AllianceTowersControlled;
}

void OutdoorPvPTF::SetAllianceTowersControlled(uint32 count)
{
    m_AllianceTowersControlled = count;
}

uint32 OutdoorPvPTF::GetHordeTowersControlled() const
{
    return m_HordeTowersControlled;
}

void OutdoorPvPTF::SetHordeTowersControlled(uint32 count)
{
    m_HordeTowersControlled = count;
}

bool OutdoorPvPTF::IsLocked() const
{
    return m_IsLocked;
}

bool OutdoorPvPTF::SetupOutdoorPvP()
{
    m_AllianceTowersControlled = 0;
    m_HordeTowersControlled = 0;

    m_IsLocked = false;
    m_LockTimer = TF_LOCK_TIME;
    m_LockTimerUpdate = 0;
    hours_left = 6;
    second_digit = 0;
    first_digit = 0;

    // add the zones affected by the pvp buff
    for (uint8 i = 0; i < OutdoorPvPTFBuffZonesNum; ++i)
        RegisterZone(OutdoorPvPTFBuffZones[i]);

    AddCapturePoint(new OPvPCapturePointTF(this, TF_TOWER_NW));
    AddCapturePoint(new OPvPCapturePointTF(this, TF_TOWER_N));
    AddCapturePoint(new OPvPCapturePointTF(this, TF_TOWER_NE));
    AddCapturePoint(new OPvPCapturePointTF(this, TF_TOWER_SE));
    AddCapturePoint(new OPvPCapturePointTF(this, TF_TOWER_S));

    return true;
}

bool OPvPCapturePointTF::Update(uint32 diff)
{
    // can update even in locked state if gathers the controlling faction
    bool canupdate = ((((OutdoorPvPTF*)m_PvP)->GetAllianceTowersControlled() > 0) && m_activePlayers[0].size() > m_activePlayers[1].size()) ||
            ((((OutdoorPvPTF*)m_PvP)->GetHordeTowersControlled() > 0) && m_activePlayers[0].size() < m_activePlayers[1].size());
    // if gathers the other faction, then only update if the pvp is unlocked
    canupdate = canupdate || !((OutdoorPvPTF*)m_PvP)->IsLocked();
    return canupdate && OPvPCapturePoint::Update(diff);
}

void OPvPCapturePointTF::ChangeState()
{
    // if changing from controlling alliance to horde
    if (m_OldState == OBJECTIVESTATE_ALLIANCE)
    {
        if (uint32 alliance_towers = ((OutdoorPvPTF*)m_PvP)->GetAllianceTowersControlled())
            ((OutdoorPvPTF*)m_PvP)->SetAllianceTowersControlled(--alliance_towers);
        sWorld->SendZoneText(OutdoorPvPTFBuffZones[0], sObjectMgr->GetSkyFireStringForDBCLocale(LANG_OPVP_TF_LOSE_A));
    }
    // if changing from controlling horde to alliance
    else if (m_OldState == OBJECTIVESTATE_HORDE)
    {
        if (uint32 horde_towers = ((OutdoorPvPTF*)m_PvP)->GetHordeTowersControlled())
            ((OutdoorPvPTF*)m_PvP)->SetHordeTowersControlled(--horde_towers);
        sWorld->SendZoneText(OutdoorPvPTFBuffZones[0], sObjectMgr->GetSkyFireStringForDBCLocale(LANG_OPVP_TF_LOSE_H));
    }

    uint32 artkit = 21;

    switch (m_State)
    {
    case OBJECTIVESTATE_ALLIANCE:
    {
        m_TowerState = TF_TOWERSTATE_A;
        artkit = 2;
        uint32 alliance_towers = ((OutdoorPvPTF*)m_PvP)->GetAllianceTowersControlled();
        if (alliance_towers < TF_TOWER_NUM)
            ((OutdoorPvPTF*)m_PvP)->SetAllianceTowersControlled(++alliance_towers);

        sWorld->SendZoneText(OutdoorPvPTFBuffZones[0], sObjectMgr->GetSkyFireStringForDBCLocale(LANG_OPVP_TF_CAPTURE_A));

        for (PlayerSet::iterator itr = m_activePlayers[0].begin(); itr != m_activePlayers[0].end(); ++itr)
            if (Player* player = ObjectAccessor::FindPlayer(*itr))
                player->AreaExploredOrEventHappens(TF_ALLY_QUEST);
        break;
    }
    case OBJECTIVESTATE_HORDE:
    {
        m_TowerState = TF_TOWERSTATE_H;
        artkit = 1;
        uint32 horde_towers = ((OutdoorPvPTF*)m_PvP)->GetHordeTowersControlled();
        if (horde_towers < TF_TOWER_NUM)
            ((OutdoorPvPTF*)m_PvP)->SetHordeTowersControlled(++horde_towers);

        sWorld->SendZoneText(OutdoorPvPTFBuffZones[0], sObjectMgr->GetSkyFireStringForDBCLocale(LANG_OPVP_TF_CAPTURE_H));

        for (PlayerSet::iterator itr = m_activePlayers[1].begin(); itr != m_activePlayers[1].end(); ++itr)
            if (Player* player = ObjectAccessor::FindPlayer(*itr))
                player->AreaExploredOrEventHappens(TF_HORDE_QUEST);
        break;
    }
    case OBJECTIVESTATE_NEUTRAL:
    case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
    case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
    case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:
    case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:
        m_TowerState = TF_TOWERSTATE_N;
        break;
    }

    GameObject* flag = HashMapHolder<GameObject>::Find(m_capturePointGUID);
    if (flag)
        flag->SetGoArtKit(artkit);

    UpdateTowerState();
}

void OPvPCapturePointTF::SendChangePhase()
{
    // send this too, sometimes the slider disappears, dunno why :(
    SendUpdateWorldState(TF_UI_TOWER_SLIDER_DISPLAY, 1);
    // send these updates to only the ones in this objective
    uint32 phase = (uint32)ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f);
    SendUpdateWorldState(TF_UI_TOWER_SLIDER_POS, phase);
    // send this too, sometimes it resets :S
    SendUpdateWorldState(TF_UI_TOWER_SLIDER_N, m_neutralValuePct);
}

class OutdoorPvP_terokkar_forest : public OutdoorPvPScript
{
    public:
        OutdoorPvP_terokkar_forest() : OutdoorPvPScript("outdoorpvp_tf") { }

        OutdoorPvP* GetOutdoorPvP() const OVERRIDE
        {
            return new OutdoorPvPTF();
        }
};

void AddSC_outdoorpvp_tf()
{
    new OutdoorPvP_terokkar_forest();
}
