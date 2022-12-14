#pragma once
#include "NavigationMap.h"
#include <string>
namespace NCL {
	namespace CSC8503 {
		struct GridNode {
			GridNode* parent;

			GridNode* connected[4];
			int		  costs[4];

			Vector3		position;

			float f;
			float g;

			int type;

			GridNode() {
				for (int i = 0; i < 4; ++i) {
					connected[i] = nullptr;
					costs[i] = 0;
				}
				f = 0;
				g = 0;
				type = 0;
				parent = nullptr;
			}
			~GridNode() {	}
		};

		class NavigationGrid : public NavigationMap	{
		public:
			NavigationGrid();
			NavigationGrid(const std::string&filename);
			~NavigationGrid();

			bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) override;
			bool FindOffsetPath(const Vector3& from, const Vector3& to, NavigationPath& outPath, float xOffset = 0.0f, float zOffset = 0.0f){
				Vector3 newFrom(from.x - xOffset, from.y, from.z - zOffset);
				Vector3 newTo(to.x - xOffset, to.y, to.z - zOffset);
				return FindPath(newFrom, newTo, outPath);
			}
			GridNode* AllNodes() {
				return allNodes;
			}

			int GridWidth() {
				return gridWidth;
			}

			int GridHeight() {
				return gridHeight;
			}

			int NodeSize() {
				return nodeSize;
			}

			void BuildNodeConnections();
				
		protected:
			bool		NodeInList(GridNode* n, std::vector<GridNode*>& list) const;
			GridNode*	RemoveBestNode(std::vector<GridNode*>& list) const;
			float		Heuristic(GridNode* hNode, GridNode* endNode) const;
			int nodeSize;
			int gridWidth;
			int gridHeight;

			GridNode* allNodes;
		};
	}
}

