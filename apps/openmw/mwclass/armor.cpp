#include "armor.hpp"

#include <components/esm/loadarmo.hpp>
#include <components/esm/loadskil.hpp>
#include <components/esm/loadgmst.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/world.hpp"
#include "../mwbase/windowmanager.hpp"

#include "../mwworld/ptr.hpp"
#include "../mwworld/actiontake.hpp"
#include "../mwworld/actionequip.hpp"
#include "../mwworld/inventorystore.hpp"
#include "../mwworld/cellstore.hpp"
#include "../mwworld/esmstore.hpp"
#include "../mwphysics/physicssystem.hpp"
#include "../mwworld/nullaction.hpp"
#include "../mwworld/containerstore.hpp"

#include "../mwrender/objects.hpp"
#include "../mwrender/renderinginterface.hpp"
#include "../mwmechanics/actorutil.hpp"

#include "../mwgui/tooltips.hpp"

namespace MWClass
{
    std::string Armor::getId (const MWWorld::Ptr& ptr) const
    {
        return ptr.get<ESM::Armor>()->mBase->mId;
    }

    void Armor::insertObjectRendering (const MWWorld::Ptr& ptr, const std::string& model, MWRender::RenderingInterface& renderingInterface) const
    {
        if (!model.empty()) {
            renderingInterface.getObjects().insertModel(ptr, model);
        }
    }

    void Armor::insertObject(const MWWorld::Ptr& ptr, const std::string& model, MWPhysics::PhysicsSystem& physics) const
    {
        // TODO: add option somewhere to enable collision for placeable objects
    }

    std::string Armor::getModel(const MWWorld::ConstPtr &ptr) const
    {
        const MWWorld::LiveCellRef<ESM::Armor> *ref = ptr.get<ESM::Armor>();

        const std::string &model = ref->mBase->mModel;
        if (!model.empty()) {
            return "meshes\\" + model;
        }
        return "";
    }

    std::string Armor::getName (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM::Armor> *ref = ptr.get<ESM::Armor>();

        return ref->mBase->mName;
    }

    boost::shared_ptr<MWWorld::Action> Armor::activate (const MWWorld::Ptr& ptr,
        const MWWorld::Ptr& actor) const
    {
        return defaultItemActivate(ptr, actor);
    }

    bool Armor::hasItemHealth (const MWWorld::Ptr& ptr) const
    {
        return true;
    }

    int Armor::getItemMaxHealth (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM::Armor> *ref = ptr.get<ESM::Armor>();

        return ref->mBase->mData.mHealth;
    }

    std::string Armor::getScript (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM::Armor> *ref = ptr.get<ESM::Armor>();

        return ref->mBase->mScript;
    }

    std::pair<std::vector<int>, bool> Armor::getEquipmentSlots (const MWWorld::Ptr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM::Armor> *ref = ptr.get<ESM::Armor>();

        std::vector<int> slots_;

        const int size = 11;

        static const int sMapping[size][2] =
        {
            { ESM::Armor::Helmet, MWWorld::InventoryStore::Slot_Helmet },
            { ESM::Armor::Cuirass, MWWorld::InventoryStore::Slot_Cuirass },
            { ESM::Armor::LPauldron, MWWorld::InventoryStore::Slot_LeftPauldron },
            { ESM::Armor::RPauldron, MWWorld::InventoryStore::Slot_RightPauldron },
            { ESM::Armor::Greaves, MWWorld::InventoryStore::Slot_Greaves },
            { ESM::Armor::Boots, MWWorld::InventoryStore::Slot_Boots },
            { ESM::Armor::LGauntlet, MWWorld::InventoryStore::Slot_LeftGauntlet },
            { ESM::Armor::RGauntlet, MWWorld::InventoryStore::Slot_RightGauntlet },
            { ESM::Armor::Shield, MWWorld::InventoryStore::Slot_CarriedLeft },
            { ESM::Armor::LBracer, MWWorld::InventoryStore::Slot_LeftGauntlet },
            { ESM::Armor::RBracer, MWWorld::InventoryStore::Slot_RightGauntlet }
        };

        for (int i=0; i<size; ++i)
            if (sMapping[i][0]==ref->mBase->mData.mType)
            {
                slots_.push_back (int (sMapping[i][1]));
                break;
            }

        return std::make_pair (slots_, false);
    }

    int Armor::getEquipmentSkill (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM::Armor> *ref = ptr.get<ESM::Armor>();

        std::string typeGmst;

        switch (ref->mBase->mData.mType)
        {
            case ESM::Armor::Helmet: typeGmst = "iHelmWeight"; break;
            case ESM::Armor::Cuirass: typeGmst = "iCuirassWeight"; break;
            case ESM::Armor::LPauldron:
            case ESM::Armor::RPauldron: typeGmst = "iPauldronWeight"; break;
            case ESM::Armor::Greaves: typeGmst = "iGreavesWeight"; break;
            case ESM::Armor::Boots: typeGmst = "iBootsWeight"; break;
            case ESM::Armor::LGauntlet:
            case ESM::Armor::RGauntlet: typeGmst = "iGauntletWeight"; break;
            case ESM::Armor::Shield: typeGmst = "iShieldWeight"; break;
            case ESM::Armor::LBracer:
            case ESM::Armor::RBracer: typeGmst = "iGauntletWeight"; break;
        }

        if (typeGmst.empty())
            return -1;

        const MWWorld::Store<ESM::GameSetting> &gmst =
            MWBase::Environment::get().getWorld()->getStore().get<ESM::GameSetting>();

        float iWeight = floor(gmst.find(typeGmst)->getFloat());

        float epsilon = 0.0005f;

        if (ref->mBase->mData.mWeight == 0)
            return ESM::Skill::Unarmored;

        if (ref->mBase->mData.mWeight <= iWeight * gmst.find ("fLightMaxMod")->getFloat() + epsilon)
            return ESM::Skill::LightArmor;

        if (ref->mBase->mData.mWeight <= iWeight * gmst.find ("fMedMaxMod")->getFloat() + epsilon)
            return ESM::Skill::MediumArmor;

        else
            return ESM::Skill::HeavyArmor;
    }

    int Armor::getValue (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM::Armor> *ref = ptr.get<ESM::Armor>();

        return ref->mBase->mData.mValue;
    }

    void Armor::registerSelf()
    {
        boost::shared_ptr<Class> instance (new Armor);

        registerClass (typeid (ESM::Armor).name(), instance);
    }

    std::string Armor::getUpSoundId (const MWWorld::Ptr& ptr) const
    {
        int es = getEquipmentSkill(ptr);
        if (es == ESM::Skill::LightArmor)
            return std::string("Item Armor Light Up");
        else if (es == ESM::Skill::MediumArmor)
            return std::string("Item Armor Medium Up");
        else
            return std::string("Item Armor Heavy Up");
    }

    std::string Armor::getDownSoundId (const MWWorld::Ptr& ptr) const
    {
        int es = getEquipmentSkill(ptr);
        if (es == ESM::Skill::LightArmor)
            return std::string("Item Armor Light Down");
        else if (es == ESM::Skill::MediumArmor)
            return std::string("Item Armor Medium Down");
        else
            return std::string("Item Armor Heavy Down");
    }

    std::string Armor::getInventoryIcon (const MWWorld::Ptr& ptr) const
    {
          MWWorld::LiveCellRef<ESM::Armor> *ref =
            ptr.get<ESM::Armor>();

        return ref->mBase->mIcon;
    }

    bool Armor::hasToolTip (const MWWorld::Ptr& ptr) const
    {
        MWWorld::LiveCellRef<ESM::Armor> *ref =
            ptr.get<ESM::Armor>();

        return (ref->mBase->mName != "");
    }

    MWGui::ToolTipInfo Armor::getToolTipInfo (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM::Armor> *ref = ptr.get<ESM::Armor>();

        MWGui::ToolTipInfo info;
        info.caption = ref->mBase->mName + MWGui::ToolTips::getCountString(ptr.getRefData().getCount());
        info.icon = ref->mBase->mIcon;

        std::string text;

        // get armor type string (light/medium/heavy)
        int armorType = getEquipmentSkill(ptr);
        std::string typeText;
        if (armorType == ESM::Skill::LightArmor)
            typeText = "#{sLight}";
        else if (armorType == ESM::Skill::MediumArmor)
            typeText = "#{sMedium}";
        else
            typeText = "#{sHeavy}";

        text += "\n#{sArmorRating}: " + MWGui::ToolTips::toString(getEffectiveArmorRating(ptr,
            MWMechanics::getPlayer()));

        int remainingHealth = getItemHealth(ptr);
        text += "\n#{sCondition}: " + MWGui::ToolTips::toString(remainingHealth) + "/"
                + MWGui::ToolTips::toString(ref->mBase->mData.mHealth);

        text += "\n#{sWeight}: " + MWGui::ToolTips::toString(ref->mBase->mData.mWeight) + " (" + typeText + ")";
        text += MWGui::ToolTips::getValueString(ref->mBase->mData.mValue, "#{sValue}");

        if (MWBase::Environment::get().getWindowManager()->getFullHelp()) {
            text += MWGui::ToolTips::getCellRefString(ptr.getCellRef());
            text += MWGui::ToolTips::getMiscString(ref->mBase->mScript, "Script");
        }

        info.enchant = ref->mBase->mEnchant;
        if (!info.enchant.empty())
            info.remainingEnchantCharge = static_cast<int>(ptr.getCellRef().getEnchantmentCharge());

        info.text = text;

        return info;
    }

    std::string Armor::getEnchantment (const MWWorld::Ptr& ptr) const
    {
        MWWorld::LiveCellRef<ESM::Armor> *ref =
            ptr.get<ESM::Armor>();

        return ref->mBase->mEnchant;
    }

    std::string Armor::applyEnchantment(const MWWorld::Ptr &ptr, const std::string& enchId, int enchCharge, const std::string& newName) const
    {
        MWWorld::LiveCellRef<ESM::Armor> *ref =
            ptr.get<ESM::Armor>();

        ESM::Armor newItem = *ref->mBase;
        newItem.mId="";
        newItem.mName=newName;
        newItem.mData.mEnchant=enchCharge;
        newItem.mEnchant=enchId;
        const ESM::Armor *record = MWBase::Environment::get().getWorld()->createRecord (newItem);
        return record->mId;
    }

    int Armor::getEffectiveArmorRating(const MWWorld::ConstPtr &ptr, const MWWorld::Ptr &actor) const
    {
        const MWWorld::LiveCellRef<ESM::Armor> *ref = ptr.get<ESM::Armor>();

        int armorSkillType = getEquipmentSkill(ptr);
        int armorSkill = actor.getClass().getSkill(actor, armorSkillType);

        const MWBase::World *world = MWBase::Environment::get().getWorld();
        int iBaseArmorSkill = world->getStore().get<ESM::GameSetting>().find("iBaseArmorSkill")->getInt();

        if(ref->mBase->mData.mWeight == 0)
            return ref->mBase->mData.mArmor;
        else
            return ref->mBase->mData.mArmor * armorSkill / iBaseArmorSkill;
    }

    std::pair<int, std::string> Armor::canBeEquipped(const MWWorld::Ptr &ptr, const MWWorld::Ptr &npc) const
    {
        MWWorld::InventoryStore& invStore = npc.getClass().getInventoryStore(npc);

        if (ptr.getCellRef().getCharge() == 0)
            return std::make_pair(0, "#{sInventoryMessage1}");

        // slots that this item can be equipped in
        std::pair<std::vector<int>, bool> slots_ = ptr.getClass().getEquipmentSlots(ptr);

        if (slots_.first.empty())
            return std::make_pair(0, "");

        if (npc.getClass().isNpc())
        {
            std::string npcRace = npc.get<ESM::NPC>()->mBase->mRace;

            // Beast races cannot equip shoes / boots, or full helms (head part vs hair part)
            const ESM::Race* race = MWBase::Environment::get().getWorld()->getStore().get<ESM::Race>().find(npcRace);
            if(race->mData.mFlags & ESM::Race::Beast)
            {
                std::vector<ESM::PartReference> parts = ptr.get<ESM::Armor>()->mBase->mParts.mParts;

                for(std::vector<ESM::PartReference>::iterator itr = parts.begin(); itr != parts.end(); ++itr)
                {
                    if((*itr).mPart == ESM::PRT_Head)
                        return std::make_pair(0, "#{sNotifyMessage13}");
                    if((*itr).mPart == ESM::PRT_LFoot || (*itr).mPart == ESM::PRT_RFoot)
                        return std::make_pair(0, "#{sNotifyMessage14}");
                }
            }
        }

        for (std::vector<int>::const_iterator slot=slots_.first.begin();
            slot!=slots_.first.end(); ++slot)
        {
            // If equipping a shield, check if there's a twohanded weapon conflicting with it
            if(*slot == MWWorld::InventoryStore::Slot_CarriedLeft)
            {
                MWWorld::ContainerStoreIterator weapon = invStore.getSlot(MWWorld::InventoryStore::Slot_CarriedRight);

                if(weapon == invStore.end())
                    return std::make_pair(1,"");

                if(weapon->getTypeName() == typeid(ESM::Weapon).name() &&
                        (weapon->get<ESM::Weapon>()->mBase->mData.mType == ESM::Weapon::LongBladeTwoHand ||
                weapon->get<ESM::Weapon>()->mBase->mData.mType == ESM::Weapon::BluntTwoClose ||
                weapon->get<ESM::Weapon>()->mBase->mData.mType == ESM::Weapon::BluntTwoWide ||
                weapon->get<ESM::Weapon>()->mBase->mData.mType == ESM::Weapon::SpearTwoWide ||
                weapon->get<ESM::Weapon>()->mBase->mData.mType == ESM::Weapon::AxeTwoHand ||
                weapon->get<ESM::Weapon>()->mBase->mData.mType == ESM::Weapon::MarksmanBow ||
                weapon->get<ESM::Weapon>()->mBase->mData.mType == ESM::Weapon::MarksmanCrossbow))
                {
                    return std::make_pair(3,"");
                }
                return std::make_pair(1,"");
            }
        }
        return std::make_pair(1,"");
    }

    boost::shared_ptr<MWWorld::Action> Armor::use (const MWWorld::Ptr& ptr) const
    {
        boost::shared_ptr<MWWorld::Action> action(new MWWorld::ActionEquip(ptr));

        action->setSound(getUpSoundId(ptr));

        return action;
    }

    MWWorld::Ptr
    Armor::copyToCellImpl(const MWWorld::Ptr &ptr, MWWorld::CellStore &cell) const
    {
        MWWorld::LiveCellRef<ESM::Armor> *ref =
            ptr.get<ESM::Armor>();

        return MWWorld::Ptr(cell.insert(ref), &cell);
    }

    int Armor::getEnchantmentPoints (const MWWorld::Ptr& ptr) const
    {
        MWWorld::LiveCellRef<ESM::Armor> *ref =
                ptr.get<ESM::Armor>();

        return ref->mBase->mData.mEnchant;
    }

    bool Armor::canSell (const MWWorld::Ptr& item, int npcServices) const
    {
        return (npcServices & ESM::NPC::Armor)
                || ((npcServices & ESM::NPC::MagicItems) && !getEnchantment(item).empty());
    }

    float Armor::getWeight(const MWWorld::Ptr &ptr) const
    {
        MWWorld::LiveCellRef<ESM::Armor> *ref =
            ptr.get<ESM::Armor>();
        return ref->mBase->mData.mWeight;
    }
}
