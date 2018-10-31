#include "hashlifetable.h"
#include "hashlifemacrocell.h"

inline std::size_t hash_pointers(const Macrocell* value)
{
	std::size_t return_value = 0;
	hash_combine(return_value, value->nw);
	hash_combine(return_value, value->ne);
	hash_combine(return_value, value->se);
	hash_combine(return_value, value->sw);
	return return_value;
}

inline std::size_t hash_pointers(const Macrocell* value0, const Macrocell* value1, const Macrocell* value2, const Macrocell* value3)
{
	std::size_t return_value = 0;
	hash_combine(return_value, value0);
	hash_combine(return_value, value1);
	hash_combine(return_value, value2);
	hash_combine(return_value, value3);
	return return_value;
}

HashLife_Table::HashLife_Table() : empty_cells(3),
	  alive_cell(new Macrocell(reinterpret_cast<Macrocell*>(0x01), nullptr, nullptr, nullptr)),
	  dead_cell(new Macrocell(nullptr, nullptr, nullptr, nullptr)), precalced_gens_exp(0),
	  survival_rules((1 << 2) | (1 << 3)), rebirth_rules((1 << 3)),
	  data_pos(262144, 0xFF), data(262144), num_of_elements(0)
{
	fill_hash_table();
}

HashLife_Table::~HashLife_Table()
{
	for(std::size_t i = 0; i < data_pos.size(); ++i)
		if(!is_empty_slot(data_pos[i]))
			delete data[i].second;
}

void HashLife_Table::resize(std::size_t new_size)
{
	empty_cells.reserve(new_size + 1);

	while(empty_cells.size() <= new_size)
	{
		Macrocell* empty_temp = empty_cells[empty_cells.size() - 1];
		empty_cells.emplace_back(Macrocell::new_macrocell(empty_temp, empty_temp,empty_temp, empty_temp));
	}
}

void HashLife_Table::clear()
{
	for(std::size_t i = 0; i < data_pos.size(); ++i)
		if(!is_empty_slot(data_pos[i]))
			delete data[i].second;

	empty_cells.resize(3);
	alive_cell = new Macrocell(reinterpret_cast<Macrocell*>(0x01), nullptr, nullptr, nullptr);
	dead_cell = new Macrocell(nullptr, nullptr, nullptr, nullptr);
	precalced_gens_exp = 0;
	data_pos.clear();
	data_pos.resize(262144, 0xFF);
	data.resize(262144);
	num_of_elements = 0;

	fill_hash_table();
}

void HashLife_Table::clear_result(Macrocell* cell, std::size_t level, std::size_t last_level)
{
	if(cell->result == nullptr || cell->ne == nullptr || cell->ne->ne == nullptr || cell->ne->ne->ne == nullptr)
			return;

	if(level > last_level)
		cell->result = nullptr;
}

void HashLife_Table::clear_results(std::size_t level)
{
	for(std::size_t i = 0; i < data.size(); ++i)
	{
		// skip empty slots
		if(!is_empty_slot(data_pos[i]))
			// clear all results (alternative: calc level of each cell using Macrocell::level() -> expensive)
			clear_result(data[i].second, empty_cells.size(), level);
	}
}

void HashLife_Table::set_rule_set(std::size_t survival_rules, std::size_t rebirth_rules)
{
	if(this->survival_rules == survival_rules && this->rebirth_rules == rebirth_rules)
		return;

	this->survival_rules = survival_rules;
	this->rebirth_rules = rebirth_rules;

	clear();
}

Macrocell* HashLife_Table::operator[](const Macrocell* key) const
{
	return get(key->nw, key->ne, key->se, key->sw);
}

Macrocell* HashLife_Table::get(const Macrocell* key0, const Macrocell* key1, const Macrocell* key2, const Macrocell* key3) const
{
	std::size_t hash_value = hash_pointers(key0, key1, key2, key3);
	const std::size_t max_size = (data_pos.size() - 1);
	std::size_t num = hash_value & max_size;

	std::size_t distance = 0;
	bool search_enabled = false;
	while(true)
	{
		if(is_empty_slot(data_pos[num]))
			return nullptr;
		else
		{
			if(distance == data_pos[num])
			{
				search_enabled = true;
				if(data[num].first == hash_value)
				{
					if(data[num].second->nw == key0 &&
							data[num].second->ne == key1 &&
							data[num].second->se == key2 &&
							data[num].second->sw == key3)
						return data[num].second;
				}
			}
			else if(search_enabled)
				return nullptr;

			distance = (distance + 1) & max_size;
			num = (num + 1) & max_size;
		}
	}
}

void HashLife_Table::insert(const Macrocell* key, Macrocell* value)
{
	insert(key->nw, key->ne, key->se, key->sw, value);
}

void HashLife_Table::insert(const Macrocell* key0, const Macrocell* key1, const Macrocell* key2, const Macrocell* key3, Macrocell* value)
{
	if(++num_of_elements > max_load_factor * data.size())
		internal_resize(data.size() << 1);

	std::size_t hash_value = hash_pointers(key0, key1, key2, key3);
	const std::size_t max_size = (data_pos.size() - 1);
	std::size_t num = hash_value & max_size;

	uint8_t distance = 0;
	while(true)
	{
		if(is_empty_slot(data_pos[num]))
		{
			data_pos[num] = distance;
			data[num].first = hash_value;
			data[num].second = value;
			return;
		}
		else
		{
			if(distance > data_pos[num])
			{
				std::swap(distance, data_pos[num]);
				std::swap(hash_value, data[num].first);
				std::swap(value, data[num].second);
				distance = 0;
				num = hash_value & max_size;
			}

			distance = (distance + 1) & max_size;
			num = (num + 1) & max_size;
		}
	}
}

void HashLife_Table::fill_hash_table()
{
	insert(dead_cell, dead_cell);
	insert(alive_cell, alive_cell);
	empty_cells[0] = dead_cell;

	std::array<Macrocell*, 16> first_level_cells;
	for(unsigned short a = 0; a < 2; ++a)
		for(unsigned short b = 0; b < 2; ++b)
			for(unsigned short c = 0; c < 2; ++c)
				for(unsigned short d = 0; d < 2; ++d)
				{
					Macrocell* temp = new Macrocell((a) ? alive_cell : dead_cell,
													(b) ? alive_cell : dead_cell,
													(c) ? alive_cell : dead_cell,
													(d) ? alive_cell : dead_cell);
					insert(temp, temp);
					first_level_cells[8 * a + 4 * b + 2 * c + d] = temp;

					if(!a && !b && !c && !d)
						empty_cells[1] = temp;
				}

	for(const auto& a : first_level_cells)
		for(const auto& b : first_level_cells)
			for(const auto& c : first_level_cells)
				for(const auto& d : first_level_cells)
				{
					Macrocell* temp = new Macrocell(a, b, c, d);
					insert(temp, temp);
					temp->result = get_second_level_result(temp);
					if(temp->population == 0)
						empty_cells[2] = temp;
				}
}

void HashLife_Table::internal_resize(std::size_t new_size)
{
	decltype(data) temp_data(new_size);
	std::fill(data_pos.begin(), data_pos.end(), 0xFF);
	data_pos.resize(new_size, 0xFF);

	const std::size_t max_size = (new_size - 1);

	for(auto& a : data)
	{
		// skip empty entries
		if(a.second == nullptr)
			continue;

		std::size_t num = a.first & max_size;

		uint8_t distance = 0;
		while(true)
		{
			if(is_empty_slot(data_pos[num]))
			{
				data_pos[num] = distance;
				temp_data[num].first = a.first;
				temp_data[num].second = a.second;
				break;
			}
			else
			{
				if(distance > data_pos[num])
				{
					std::swap(distance, data_pos[num]);
					a.swap(temp_data[num]);
					distance = 0;
					num = a.first & max_size;
				}

				++distance;
				num = (num + 1) & max_size;
			}
		}
	}

	data = std::move(temp_data);
}

Macrocell* HashLife_Table::calc_gen(std::size_t bitmask)
{
	if(bitmask == 0)
		return dead_cell;
	int self = (bitmask >> 5) & 1;
	bitmask &= 0x757 ; // mask out bits we don't care about

	std::size_t alive_neighbors = 0;
	while(bitmask)
	{
		++alive_neighbors;
		bitmask &= bitmask - 1 ; // clear least significant bit
	}

	// if cell is alive, try survival_rules
	if(self)
	{
		if(survival_rules & (1 << alive_neighbors))
			return alive_cell;
		else
			return dead_cell;
	}
	// if cell is dead, try rebirth_rules
	else if(rebirth_rules & (1 << alive_neighbors))
		return alive_cell;
	else
		return dead_cell;
}

Macrocell* HashLife_Table::get_second_level_result(Macrocell* second_level)
{
	// get all bits of macrocell
	std::size_t allbits = 0 ;
	for(std::size_t y = 0; y < 4; ++y)
		for(std::size_t x = 0; x < 4; ++x)
			allbits = (allbits << 1) | second_level->get_state(x, y, 2);

	// requires level 1 macrocells in table
	return get(calc_gen(allbits >> 5), calc_gen(allbits >> 4), calc_gen(allbits), calc_gen(allbits >> 1));
}