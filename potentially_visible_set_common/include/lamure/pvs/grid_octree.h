// Copyright (c) 2014 Bauhaus-Universitaet Weimar
// This Software is distributed under the Modified BSD License, see license.txt.
//
// Virtual Reality and Visualization Research Group 
// Faculty of Media, Bauhaus-Universitaet Weimar
// http://www.uni-weimar.de/medien/vr

#ifndef LAMURE_PVS_GRID_OCTREE_H
#define LAMURE_PVS_GRID_OCTREE_H

#include "lamure/pvs/grid.h"
#include "lamure/pvs/grid_octree_node.h"

#include <mutex>

namespace lamure
{
namespace pvs
{

class grid_octree : public grid
{
public:
	grid_octree();
	grid_octree(const size_t& octree_depth, const double& size, const scm::math::vec3d& position_center, const std::vector<node_t>& ids);
	virtual ~grid_octree();

	virtual size_t get_cell_count() const;

	virtual const view_cell* get_cell_at_index(const size_t& index) const;
	virtual const view_cell* get_cell_at_position(const scm::math::vec3d& position) const;

	virtual void set_cell_visibility(const size_t& cell_index, const model_t& model_id, const node_t& node_id, const bool& visibility);

	virtual void save_grid_to_file(const std::string& file_path) const;
	virtual void save_visibility_to_file(const std::string& file_path) const;

	virtual bool load_grid_from_file(const std::string& file_path);
	virtual bool load_visibility_from_file(const std::string& file_path);

	virtual model_t get_num_models() const;
	virtual node_t get_num_nodes(const model_t& model_id) const;

protected:
	void create_grid(grid_octree_node* node, size_t depth);
	void compute_index_access();

	size_t cell_count_recursive(const grid_octree_node* node) const;
	grid_octree_node* find_cell_by_index_recursive(const grid_octree_node* node, const size_t& index, size_t base_value);
	const grid_octree_node* find_cell_by_index_recursive_const(const grid_octree_node* node, const size_t& index, size_t base_value) const;
	const grid_octree_node* find_cell_by_position_recursive_const(const grid_octree_node* node, const scm::math::vec3d& position) const;

	// Grid is managed as an octree, ech node manages its 8 children. Each node is accessed (in)directly via the root node.
	grid_octree_node* root_node_;
	double bounding_size_;
	std::vector<node_t> ids_;

	mutable std::mutex mutex_;

	// Used to improve performance of get_cell_at_index().
	std::vector<view_cell*> cells_by_indices_;
};

}
}

#endif
