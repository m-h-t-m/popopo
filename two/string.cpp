#include<iostream>
#include<cstring>
class String{
	private:
		char*  data;
		int len;
	public: 
		String(const char* str = nullptr){
		if(str == nullptr || str[0] == '\0'){
			data = new char[1];
			data[0] = '\0';
			len =0;
		}else{
			len = std::strlen(str);
			data = new char[len +1];
			std::strcpy(data, str);
		}
		}
		~String() {
			delete[] data;
			data = nullptr;
		}
		int length() const{
			return len;
		}
		const char* c_str() const{
			return data;
		}
};

int main()
{
	String s1("Hello, World");
	std::cout << "s1: "<<s1.c_str() <<std::endl;
	std::cout << "Length of s1: " << s1.length() <<std::endl;
	{
		String s4("Temporary");
		std::cout << "s4: " << s4.c_str() <<std::endl;
	}
	std::cout << "s4 die" << std::endl;
}
