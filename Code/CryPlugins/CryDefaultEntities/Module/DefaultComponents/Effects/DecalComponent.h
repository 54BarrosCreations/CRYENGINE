#pragma once

#include <CryRenderer/IRenderer.h>
#include <CryRenderer/IShader.h>

#include <CrySchematyc/ResourceTypes.h>
#include <CrySchematyc/MathTypes.h>

namespace Cry
{
	namespace DefaultComponents
	{
		class CDecalComponent
			: public IEntityComponent
		{
		public:
			virtual ~CDecalComponent() {}

			// IEntityComponent
			virtual void Initialize() override
			{
				if (m_bAutoSpawn)
				{
					Spawn();
				}
				else
				{
					Remove();
				}
			}

			virtual void Run(Schematyc::ESimulationMode simulationMode) override;

			virtual void ProcessEvent(SEntityEvent& event) override;
			virtual uint64 GetEventMask() const override;
			// ~IEntityComponent

			static void ReflectType(Schematyc::CTypeDesc<CDecalComponent>& desc);

			static CryGUID& IID()
			{
				static CryGUID id = "{26C5856F-34BC-43FE-BE36-2D276D082C96}"_cry_guid;
				return id;
			}

			void Spawn()
			{
				if (m_materialFileName.value.size() > 0 && gEnv->p3DEngine->GetMaterialManager()->FindMaterial(m_materialFileName.value) != nullptr)
				{
					IDecalRenderNode* pRenderNode = static_cast<IDecalRenderNode*>(m_pEntity->GetSlotRenderNode(GetEntitySlotId()));
					
					// Make sure that the component has a slot so the transform can be modified by designers
					if (pRenderNode == nullptr)
					{
						pRenderNode = static_cast<IDecalRenderNode*>(gEnv->p3DEngine->CreateRenderNode(eERType_Decal));
						m_pEntity->SetSlotRenderNode(GetOrMakeEntitySlotId(), pRenderNode);
					}

					bool bSelected, bHighlighted;
					m_pEntity->GetEditorObjectInfo(bSelected, bHighlighted);

					uint64 renderFlags = 0;

					if (bSelected)
					{
						renderFlags |= ERF_SELECTED;
					}

					if (m_pEntity->IsHidden())
					{
						renderFlags |= ERF_HIDDEN;
					}

					pRenderNode->SetRndFlags(renderFlags);
						
					SDecalProperties decalProperties;
					decalProperties.m_projectionType = m_projectionType;

					const Matrix34& slotTransform = m_pEntity->GetSlotWorldTM(GetEntitySlotId());

					// Get normalized rotation (remove scaling)
					Matrix33 rotation(slotTransform);
					if (m_projectionType != SDecalProperties::ePlanar)
					{
						rotation.SetRow(0, rotation.GetRow(0).GetNormalized());
						rotation.SetRow(1, rotation.GetRow(1).GetNormalized());
						rotation.SetRow(2, rotation.GetRow(2).GetNormalized());
					}

					decalProperties.m_pos = slotTransform.TransformPoint(Vec3(0, 0, 0));
					decalProperties.m_normal = slotTransform.TransformVector(Vec3(0, 0, 1));
					decalProperties.m_pMaterialName = m_materialFileName.value.c_str();
					decalProperties.m_radius = m_projectionType != SDecalProperties::ePlanar ? decalProperties.m_normal.GetLength() : 1;
					decalProperties.m_explicitRightUpFront = rotation;
					decalProperties.m_sortPrio = m_sortPriority;
					decalProperties.m_deferred = true;
					decalProperties.m_depth = m_depth;
					pRenderNode->SetDecalProperties(decalProperties);

					pRenderNode->SetMatrix(slotTransform);

					m_bSpawned = true;

					m_pEntity->UpdateComponentEventMask(this);
				}
				else
				{
					Remove();
				}
			}

			void Remove()
			{
				FreeEntitySlot();

				m_bSpawned = false;

				m_pEntity->UpdateComponentEventMask(this);
			}

			void EnableAutomaticSpawn(bool bEnable) { m_bAutoSpawn = bEnable; }
			bool IsAutomaticSpawnEnabled() const { return m_bAutoSpawn; }

			void EnableAutomaticMove(bool bEnable) { m_bAutoSpawn = bEnable; }
			bool IsAutomaticMoveEnabled() const { return m_bAutoSpawn; }

			void SetProjectionType(SDecalProperties::EProjectionType type) { m_projectionType = type; }
			SDecalProperties::EProjectionType GetProjectionType() const { return m_projectionType; }

			void SetSortPriority(uint8 priority) { m_sortPriority = (int)priority; }
			uint8 GetSortPriority() const { return m_sortPriority; }

			void SetDepth(float depth) { m_depth = depth; }
			float GetDepth() const { return m_depth; }

			virtual void SetMaterialFileName(const char* szPath);
			const char* GetMaterialFileName() const { return m_materialFileName.value.c_str(); }

		protected:
			bool m_bAutoSpawn = true;
			bool m_bFollowEntityAfterSpawn = true;
			
			SDecalProperties::EProjectionType m_projectionType = SDecalProperties::ePlanar;
			Schematyc::Range<0, 255, int> m_sortPriority = 16;
			Schematyc::Range<0, 10> m_depth = 1.f;

			bool m_bSpawned = false;

			Schematyc::MaterialFileName m_materialFileName;
		};
	}
}