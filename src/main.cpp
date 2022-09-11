#include <iostream>
#include "any.hpp"

int main()
{
	auto a = experimental::make_any<std::string>("plain text");

	std::cout << "any value: " << *experimental::any_cast<std::string>(&a) << '\n';
	*experimental::any_cast<std::string>(&a) = "other plain text";
	std::cout << "any value: " << *experimental::any_cast<std::string>(&a) << '\n';
}