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

#ifndef _MODELINSTANCE_H_
#define _MODELINSTANCE_H_

#include <G3D/Matrix3.h>
#include <G3D/Vector3.h>
#include <G3D/AABox.h>
#include <G3D/Ray.h>

#include "Define.h"

namespace VMAP
{
    class WorldModel;
    struct AreaInfo;
    struct LocationInfo;

    enum ModelFlags
    {
        MOD_M2 = 1,
        MOD_WORLDSPAWN = 1<<1,
        MOD_HAS_BOUND = 1<<2
    };

    class ModelSpawn
    {
        public:
            //mapID, tileX, tileY, Flags, ID, Pos, Rot, Scale, Bound_lo, Bound_hi, name
            uint32 flags;
            uint16 adtId;
            uint32 ID;
            G3D::Vector3 iPos;
            G3D::Vector3 iRot;
            float iScale;
            G3D::AABox iBound;
            std::string name;
            bool operator==(const ModelSpawn &other) const { return ID == other.ID; }
            //uint32 hashCode() const { return ID; }
            // temp?
            const G3D::AABox& getBounds() const { return iBound; }

            static bool readFromFile(FILE* rf, ModelSpawn &spawn);
            static bool writeToFile(FILE* rw, const ModelSpawn &spawn);
    };

    class ModelInstance: public ModelSpawn
    {
        public:
            ModelInstance() : iInvScale(0.0f), iModel(0) { }
            ModelInstance(const ModelSpawn &spawn, WorldModel* model);
            void setUnloaded() { iModel = 0; }
            bool intersectRay(const G3D::Ray& pRay, float& pMaxDist, bool pStopAtFirstHit) const;
            void intersectPoint(const G3D::Vector3& p, AreaInfo &info) const;
            bool GetLocationInfo(const G3D::Vector3& p, LocationInfo &info) const;
            bool GetLiquidLevel(const G3D::Vector3& p, LocationInfo &info, float &liqHeight) const;
        protected:
            G3D::Matrix3 iInvRot;
            float iInvScale;
            WorldModel* iModel;
        public:
            WorldModel* getWorldModel();
    };
} // namespace VMAP

#endif // _MODELINSTANCE
