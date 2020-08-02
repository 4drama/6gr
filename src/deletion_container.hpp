#ifndef DELETION_CONTAINER_HPP
#define DELETION_CONTAINER_HPP

#include <SFML/Graphics.hpp>

#include <memory>
#include <list>
#include <string>

template<typename T>
class deferred_deletion_container{
public:
	void add_pointer(std::shared_ptr<T> ptr);
	void update();
private:
	std::list<std::shared_ptr<T>> pointers;
	std::list<std::shared_ptr<T>> to_dell;
};

template<typename T>
void deferred_deletion_container<T>::update(){
	to_dell.clear();

	for(auto it = pointers.begin(); it != pointers.end();){
		if(it->use_count() == 1){
			to_dell.emplace_back(*it);
			it = pointers.erase(it);
		} else {
			++it;
		}
	}
}

template<typename T>
void deferred_deletion_container<T>::add_pointer(std::shared_ptr<T> ptr){
	pointers.emplace_back(ptr);
}

inline std::shared_ptr<sf::Text> create_text(
	deferred_deletion_container<sf::Text> *text_delete_contaier,
	const sf::String &string, const sf::Font &font, unsigned int characterSize){
	std::shared_ptr<sf::Text> res =
		std::make_shared<sf::Text>(string, font, characterSize);
	text_delete_contaier->add_pointer(res);
	return res;
}

#endif
