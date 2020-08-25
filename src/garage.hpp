#ifndef GARAGE_HPP
#define GARAGE_HPP

#include "deletion_container.hpp"

#include <utility>
#include <memory>
#include <map>
#include <functional>

class item;
class part_of_mech;

enum class items_id{
	ROCKET_BASE,
	RADIATOR_75,
	ACCUM_50,
	LEGS_BASE,
	ENGINE_BASE,
	TANK_BASE,
	COOLING_BASE,


	RADIATOR_200,
	ACCUM_100,
};

struct item_info{
	std::string name;
	float weight;
	uint32_t slots;
	std::function<std::shared_ptr<item>(deferred_deletion_container<sf::Text>*,
		const part_of_mech*)> create_func;
};

class item_db{
	using id_type = uint32_t;
public:
	item_db();

	std::shared_ptr<item> create(
		deferred_deletion_container<sf::Text> *text_delete_contaier,
		const part_of_mech *part_ptr, id_type id);

	item_info info(id_type id);
private:
	std::map<id_type, item_info> db;

	template<typename item_type, typename ...Args>
	void add(items_id id, std::string name, float weight, uint32_t slots, Args... args);

};

class garage{
	using quantity_type = uint32_t;
	using id_type = uint32_t;
public:
	void put_item(id_type id, quantity_type quantity);

	std::shared_ptr<item> take_item(item_db *item_db_ptr,
		deferred_deletion_container<sf::Text> *text_delete_contaier,
		const part_of_mech *part_ptr, id_type id);

	const std::map<id_type, quantity_type>& get_content() const noexcept{
		return this->items;};
private:
	std::map<id_type, quantity_type> items;
};

inline std::shared_ptr<item> item_db::create(
	deferred_deletion_container<sf::Text> *text_delete_contaier,
	const part_of_mech *part_ptr, id_type id){

	return db[id].create_func(text_delete_contaier, part_ptr);
}

inline item_info item_db::info(id_type id){
	return db[id];
}

#endif
