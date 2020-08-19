#include "garage.hpp"

#include "items.hpp"

item_db::item_db(){
	db[0] = item_info{"Rocket", 10, 3, [info = &db[0]](
		deferred_deletion_container<sf::Text>* text_delete_contaier,
		const part_of_mech* part_ptr) -> std::shared_ptr<item>{
		return std::make_shared<weapon>(text_delete_contaier,
			part_ptr, info->name, info->weight, info->slots, 15000);
	}};
	// TO DO other items
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
