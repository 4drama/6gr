#include "garage.hpp"

#include "items.hpp"

template<typename item_type, typename ...Args>
void item_db::add(items_id id, std::string name, float weight,
	uint32_t slots, Args... args){

	this->db[(int)id] = item_info{item_type::special, name, weight, slots,
		[id, info = &db[(int)id], args...](
		deferred_deletion_container<sf::Text>* text_delete_contaier,
		const part_of_mech* part_ptr) -> std::shared_ptr<item>{
		return std::make_shared<item_type>((int)id, text_delete_contaier,
			part_ptr, info->name, info->weight, info->slots, args...);
	}};;
};

item_db::item_db(){
	this->add<weapon>(items_id::ROCKET_BASE, "Rocket", 10, 3, 15000);
	this->add<radiator>(items_id::RADIATOR_75, "Radiator", 5, 2, 75);
	this->add<accumulator>(items_id::ACCUM_50, "Accumulator", 5, 2, 50);
	this->add<legs>(items_id::LEGS_BASE, "Legs", 20, 5);
	this->add<engine>(items_id::ENGINE_BASE, "Engine", 30, 5, 70);
	this->add<tank>(items_id::TANK_BASE, "Tank", 5, 2, 20);
	this->add<cooling_system>(items_id::COOLING_BASE, "Cooling system", 5, 2, 10.0f, 1.25f);

	this->add<radiator>(items_id::RADIATOR_200, "Radiator2", 10, 3, 200);
	this->add<accumulator>(items_id::ACCUM_100, "Accumulator2", 10, 3, 100);
}

void garage::put_item(id_type id, quantity_type quantity){
	auto item_it = items.find(id);
	if(item_it != items.end()){
		item_it->second += quantity;
	} else {
		items[id] = quantity;
	}
}

std::shared_ptr<item> garage::take_item(item_db *item_db_ptr,
	deferred_deletion_container<sf::Text> *text_delete_contaier,
	const part_of_mech *part_ptr, id_type id){
	auto item_it = items.find(id);

	std::shared_ptr<item> item = nullptr;
	if(item_it != items.end()){
		if(item_it->second > 0){
			--item_it->second;
			item = item_db_ptr->create(text_delete_contaier, part_ptr, id);
		}

		if(item_it->second == 0)
			items.erase(item_it);
	}
	return item;
};
