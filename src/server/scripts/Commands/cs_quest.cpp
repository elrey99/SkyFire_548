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

/* ScriptData
Name: quest_commandscript
%Complete: 100
Comment: All quest related commands
Category: commandscripts
EndScriptData */

#include "Chat.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ReputationMgr.h"
#include "ScriptMgr.h"

class quest_commandscript : public CommandScript
{
public:
    quest_commandscript() : CommandScript("quest_commandscript") { }

    std::vector<ChatCommand> GetCommands() const OVERRIDE
    {
        static std::vector<ChatCommand> questCommandTable =
        {
            { "add",      rbac::RBAC_PERM_COMMAND_QUEST_ADD,      false, &HandleQuestAdd,      "", },
            { "complete", rbac::RBAC_PERM_COMMAND_QUEST_COMPLETE, false, &HandleQuestComplete, "", },
            { "remove",   rbac::RBAC_PERM_COMMAND_QUEST_REMOVE,   false, &HandleQuestRemove,   "", },
            { "reward",   rbac::RBAC_PERM_COMMAND_QUEST_REWARD,   false, &HandleQuestReward,   "", },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "quest", rbac::RBAC_PERM_COMMAND_QUEST,  false, NULL, "", questCommandTable },
        };
        return commandTable;
    }

    static bool HandleQuestAdd(ChatHandler* handler, const char* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // .addquest #entry'
        // number or [name] Shift-click form |color|Hquest:quest_id:quest_level|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hquest");
        if (!cId)
            return false;

        uint32 entry = atol(cId);

        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);

        if (!quest)
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // check item starting quest (it can work incorrectly if added without item in inventory)
        ItemTemplateContainer const* itc = sObjectMgr->GetItemTemplateStore();
        ItemTemplateContainer::const_iterator result = find_if (itc->begin(), itc->end(), Finder<uint32, ItemTemplate>(entry, &ItemTemplate::StartQuest));

        if (result != itc->end())
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_STARTFROMITEM, entry, result->second.ItemId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // ok, normal (creature/GO starting) quest
        if (player->CanAddQuest(quest, true))
        {
            player->AddQuest(quest, NULL);

            if (player->CanCompleteQuest(entry))
                player->CompleteQuest(entry);
        }

        return true;
    }

    static bool HandleQuestRemove(ChatHandler* handler, const char* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // .removequest #entry'
        // number or [name] Shift-click form |color|Hquest:quest_id:quest_level|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hquest");
        if (!cId)
            return false;

        uint32 entry = atol(cId);

        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);

        if (!quest)
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // remove all quest entries for 'entry' from quest log
        for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 logQuest = player->GetQuestSlotQuestId(slot);
            if (logQuest == entry)
            {
                player->SetQuestSlot(slot, 0);

                // we ignore unequippable quest items in this case, its' still be equipped
                player->TakeQuestSourceItem(logQuest, false);

                if (quest->HasFlag(QUEST_FLAGS_FLAGS_PVP))
                {
                    player->pvpInfo.IsHostile = player->pvpInfo.IsInHostileArea || player->HasPvPForcingQuest();
                    player->UpdatePvPState();
                }
            }
        }

        player->RemoveActiveQuest(entry);
        player->RemoveRewardedQuest(entry);

        handler->SendSysMessage(LANG_COMMAND_QUEST_REMOVED);
        return true;
    }

    static bool HandleQuestComplete(ChatHandler* handler, const char* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // .quest complete #entry
        // number or [name] Shift-click form |color|Hquest:quest_id:quest_level|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hquest");
        if (!cId)
            return false;

        uint32 entry = atol(cId);

        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);

        // If player doesn't have the quest
        if (!quest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        for (QuestObjectiveSet::const_iterator citr = quest->m_questObjectives.begin(); citr != quest->m_questObjectives.end(); ++citr)
        {
            QuestObjective const* questObjective = *citr;
            switch (questObjective->Type)
            {
                case QUEST_OBJECTIVE_TYPE_NPC:
                {
                    if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(questObjective->ObjectId))
                        for (uint32 i = 0; i < uint32(questObjective->Amount); i++)
                            player->KilledMonster(creatureInfo, 0);

                    break;
                }
                case QUEST_OBJECTIVE_TYPE_GO:
                {
                    if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(questObjective->ObjectId))
                        for (uint32 i = 0; i < uint32(questObjective->Amount); i++)
                            player->KillCreditGO(questObjective->ObjectId, 0);

                    break;
                }
                case QUEST_OBJECTIVE_TYPE_ITEM:
                {
                    ItemPosCountVec dest;
                    uint32 currentCounter = player->GetItemCount(questObjective->ObjectId, true);

                    uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, questObjective->ObjectId, uint32(questObjective->Amount) - currentCounter);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, questObjective->ObjectId, true);
                        player->SendNewItem(item, uint32(questObjective->Amount) - currentCounter, true, false);
                    }

                    player->SendQuestUpdateAddCredit(quest, questObjective, ObjectGuid(0), currentCounter, uint32(questObjective->Amount) - currentCounter);

                    break;
                }
                case QUEST_OBJECTIVE_TYPE_FACTION_REP:
                case QUEST_OBJECTIVE_TYPE_FACTION_REP2:
                {
                    if (player->GetReputationMgr().GetReputation(questObjective->ObjectId) < questObjective->Amount)
                        if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(questObjective->ObjectId))
                            player->GetReputationMgr().SetReputation(factionEntry, questObjective->Amount);

                    break;
                }
                case QUEST_OBJECTIVE_TYPE_MONEY:
                {
                    player->ModifyMoney(uint64(questObjective->Amount));
                    break;
                }
                default:
                    break;
            }

            //player->SendQuestUpdateAddCredit();
        }

        player->CompleteQuest(entry);
        return true;
    }

    static bool HandleQuestReward(ChatHandler* handler, char const* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // .quest reward #entry
        // number or [name] Shift-click form |color|Hquest:quest_id:quest_level|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hquest");
        if (!cId)
            return false;

        uint32 entry = atol(cId);

        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);

        // If player doesn't have the quest
        if (!quest || player->GetQuestStatus(entry) != QUEST_STATUS_COMPLETE)
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->RewardQuest(quest, 0, player);
        return true;
    }
};

void AddSC_quest_commandscript()
{
    new quest_commandscript();
}
