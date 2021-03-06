/*
 * Originally written by Xinef - Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "azjol_nerub.h"

enum Spells
{
    SPELL_SUBBOSS_AGGRO_TRIGGER         = 52343,
    SPELL_SWARM                         = 52440,
    SPELL_MIND_FLAY                     = 52586,
    SPELL_CURSE_OF_FATIGUE              = 52592,
    SPELL_FRENZY                        = 28747
};

enum Events
{
    EVENT_KRIK_START_WAVE               = 1,
    EVENT_KRIK_SUMMON                   = 2,
    EVENT_KRIK_MIND_FLAY                = 3,
    EVENT_KRIK_CURSE                    = 4,
    EVENT_KRIK_HEALTH_CHECK             = 5,
    EVENT_KRIK_ENTER_COMBAT             = 6,
    EVENT_KILL_TALK                     = 7,
    EVENT_CALL_ADDS                     = 8,
    EVENT_KRIK_CHECK_EVADE              = 9
};

enum Npcs
{
    NPC_WATCHER_NARJIL                  = 28729,
    NPC_WATCHER_GASHRA                  = 28730,
    NPC_WATCHER_SILTHIK                 = 28731,
    NPC_WARRIOR                         = 28732,
    NPC_SKIRMISHER                      = 28734,
    NPC_SHADOWCASTER                    = 28733
};

enum Yells
{
    SAY_AGGRO                           = 0,
    SAY_SLAY                            = 1,
    SAY_DEATH                           = 2,
    SAY_SWARM                           = 3,
    SAY_PREFIGHT                        = 4,
    SAY_SEND_GROUP                      = 5
};

class boss_krik_thir : public CreatureScript
{
    public:
        boss_krik_thir() : CreatureScript("boss_krik_thir") { }

        struct boss_krik_thirAI : public BossAI
        {
            boss_krik_thirAI(Creature* creature) : BossAI(creature, DATA_KRIKTHIR_THE_GATEWATCHER_EVENT)
            {
                _initTalk = false;
            }

            EventMap events2;
            bool _initTalk;

            void Reset()
            {
                BossAI::Reset();
                events2.Reset();

                me->SummonCreature(NPC_WATCHER_NARJIL, 508.57f, 666.376f, 776.374f, 0.95f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                me->SummonCreature(NPC_SHADOWCASTER, 504.94f, 664.52f, 776.89f, 1.295f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                me->SummonCreature(NPC_WARRIOR, 506.75f, 670.7f, 776.24f, 0.92f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);   
                me->SummonCreature(NPC_WATCHER_GASHRA, 526.8f, 663.97f, 775.713f, 1.46f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                me->SummonCreature(NPC_SKIRMISHER, 524.25f, 668.02f, 775.65f, 1.24f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                me->SummonCreature(NPC_WARRIOR, 530.1f, 667.73f, 775.61f, 1.68f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                me->SummonCreature(NPC_WATCHER_SILTHIK, 546.147f, 663.203f, 776.763f, 1.759f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                me->SummonCreature(NPC_SKIRMISHER, 547.83f, 667.96f, 776.15f, 2.05f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                me->SummonCreature(NPC_SHADOWCASTER, 549.76f, 664.32f, 776.76f, 2.12f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
            }

            void MoveInLineOfSight(Unit *who)
            {
                if (who->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (!_initTalk)
                {
                    _initTalk = true;
                    Talk(SAY_PREFIGHT);
                }

                if (events.Empty() && who->GetPositionZ() < 785.0f)
                {
                    events2.ScheduleEvent(EVENT_KRIK_START_WAVE, 10000);
                    events2.ScheduleEvent(EVENT_KRIK_START_WAVE, 40000);
                    events2.ScheduleEvent(EVENT_KRIK_START_WAVE, 70000);
                    events2.ScheduleEvent(EVENT_KRIK_ENTER_COMBAT, 100000);
                    events2.ScheduleEvent(EVENT_KRIK_CHECK_EVADE, 5000);

                    events.ScheduleEvent(EVENT_KRIK_HEALTH_CHECK, 1000);
                    events.ScheduleEvent(EVENT_KRIK_MIND_FLAY, 13000);
                    events.ScheduleEvent(EVENT_KRIK_SUMMON, 17000);
                    events.ScheduleEvent(EVENT_KRIK_CURSE, 8000);
                    events.ScheduleEvent(EVENT_CALL_ADDS, 1000);
                    me->setActive(true);
                }
            }

            uint32 GetData(uint32 data) const
            {
                if (data == me->GetEntry())
                    return summons.HasEntry(NPC_WATCHER_NARJIL) && summons.HasEntry(NPC_WATCHER_GASHRA) && summons.HasEntry(NPC_WATCHER_SILTHIK);
                return 0;
            }

            void EnterCombat(Unit* who)
            {
                BossAI::EnterCombat(who);
                Talk(SAY_AGGRO);
                events2.Reset();
            }

            void JustDied(Unit* killer)
            {
                BossAI::JustDied(killer);
                Talk(SAY_DEATH);
            }

            void KilledUnit(Unit* )
            {
                if (events.GetNextEventTime(EVENT_KILL_TALK) == 0)
                {
                    Talk(SAY_SLAY);
                    events.ScheduleEvent(EVENT_KILL_TALK, 6000);
                }
            }

            void JustSummoned(Creature* summon)
            {
                summon->SetNoCallAssistance(true);
                summons.Summon(summon);
            }

            void SummonedCreatureDies(Creature* summon, Unit*)
            {
                summons.Despawn(summon);
            }

            void UpdateAI(uint32 diff)
            {
                events2.Update(diff);
                switch (events2.ExecuteEvent())
                {
                    case EVENT_KRIK_START_WAVE:
                        me->CastCustomSpell(SPELL_SUBBOSS_AGGRO_TRIGGER, SPELLVALUE_MAX_TARGETS, 1, me, true);
                        Talk(SAY_SEND_GROUP);
                        break;
                    case EVENT_KRIK_ENTER_COMBAT:
                        me->SetInCombatWithZone();
                        break;
                    case EVENT_KRIK_CHECK_EVADE:
                        if (!SelectTargetFromPlayerList(100.0f))
                        {
                            EnterEvadeMode();
                            return;
                        }
                        events2.ScheduleEvent(EVENT_KRIK_CHECK_EVADE, 5000);
                        break;
                }

                if (!UpdateVictim())
                    return;

                events.Update(diff);
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                switch (events.ExecuteEvent())
                {
                    case EVENT_KRIK_HEALTH_CHECK:
                        if (HealthBelowPct(10))
                        {
                            events.PopEvent();
                            me->CastSpell(me, SPELL_FRENZY, true);
                            break;
                        }
                        events.ScheduleEvent(EVENT_KRIK_HEALTH_CHECK, 1000);
                        break;
                    case EVENT_KRIK_SUMMON:
                        Talk(SAY_SWARM);
                        me->CastSpell(me, SPELL_SWARM, false);
                        events.ScheduleEvent(EVENT_KRIK_SUMMON, 20000);
                        break;
                    case EVENT_KRIK_MIND_FLAY:
                        me->CastSpell(me->GetVictim(), SPELL_MIND_FLAY, false);
                        events.ScheduleEvent(EVENT_KRIK_MIND_FLAY, 15000);
                        break;
                    case EVENT_KRIK_CURSE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                            me->CastSpell(target, SPELL_CURSE_OF_FATIGUE, true);
                        events.ScheduleEvent(EVENT_KRIK_CURSE, 10000);
                        break;
                    case EVENT_CALL_ADDS:
                        summons.DoZoneInCombat();
                        break;
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI *GetAI(Creature *creature) const
        {
            return new boss_krik_thirAI(creature);
        }
};

class achievement_watch_him_die : public AchievementCriteriaScript
{
    public:
        achievement_watch_him_die() : AchievementCriteriaScript("achievement_watch_him_die")
        {
        }

        bool OnCheck(Player* /*player*/, Unit* target)
        {
            if (!target)
                return false;

            return target->GetAI()->GetData(target->GetEntry());
        }
};

void AddSC_boss_krik_thir()
{
    new boss_krik_thir();
    new achievement_watch_him_die();
}
