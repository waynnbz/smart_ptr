#include <stdexcept>
#include <iostream>

//wrapper class over a pointer
template <typename T>
class smart_ptr {
public:
	//default constructor
	smart_ptr();
	~smart_ptr();

	//why and when should we use "explicit"? sof said get into the habit
	//of using explicit for unary constructors and those only?
	explicit smart_ptr(T* &raw_ptr);
	explicit smart_ptr(T* &&raw_ptr) noexcept;


	smart_ptr(const smart_ptr&  rhs);
	smart_ptr(smart_ptr&& rhs) noexcept;


	smart_ptr& operator=(const smart_ptr& rhs);
	smart_ptr& operator=(smart_ptr&& rhs) noexcept;

	bool clone();

	int ref_count() const;

	T& operator*();
	T* operator->();


private:
	T* ptr_;
	int* ref_;

	void delPtr();
};

class null_ptr_exception : public std::runtime_error //exception class
{
public:
	null_ptr_exception(const char *str)
		: runtime_error(str) {}; //why no more std??
};


//---------------------------Constructors-----------------------

template <typename T>
smart_ptr<T>::smart_ptr() : ptr_(nullptr), ref_(new int{}) {} //default c'tor


template <typename T>
smart_ptr<T>::~smart_ptr()  //destructor
{
	delPtr();
}


template <typename T>
smart_ptr<T>::smart_ptr(T* &raw_ptr) //L-value c'tor
{
	ptr_ = raw_ptr;
	ref_ = new int{ 1 };
}

template <typename T>
smart_ptr<T>::smart_ptr(T* &&raw_ptr) noexcept //R-value c'tor
{
	//std::cerr << "R c'tor" << std::endl;
	ptr_ = std::move(raw_ptr);
	ref_ = new int{ 1 };
	//raw_ptr = nullptr;
}

template <typename T>
smart_ptr<T>::smart_ptr(const smart_ptr& rhs)  //L-value SP c'tor
	:ptr_(rhs.ptr_), ref_(rhs.ref_) //aint them private data???
{
	++*ref_;
}

template <typename T>
smart_ptr<T>::smart_ptr(smart_ptr&& rhs) noexcept //R-value SP c'tor
	: ptr_(rhs.ptr_), ref_(rhs.ref_)
{
	//std::cout << "called r-v C'tor\n";
	rhs.ptr_ = rhs.ref_ = nullptr;
}


//----------------------------Operators---------------------------

template <typename T>	//L-value assignment operator
smart_ptr<T>& smart_ptr<T>::operator= (const smart_ptr& rhs)
{
	if (this != &rhs) {
		this->ptr_ = rhs.ptr_;
		this->ref_ = rhs.ref_;
		++*ref_;
	}
	return *this;
}

template <typename T>	//R-value assignment operator
smart_ptr<T>& smart_ptr<T>::operator= (smart_ptr&& rhs) noexcept
{
	//std::cout << "called r-v assign \n";
	if (this != rhs) {
		if (ref_ != 0)	delPtr();
		this->ptr_ = rhs.ptr_;
		this->ref_ = rhs.ref_;
		rhs.ptr_ = rhs.ref_ = nullptr;
	}
	return *this;
}

template<typename T>
T& smart_ptr<T>::operator*() { //asterisk operator
	try {
		if (ptr_ == nullptr)
			throw null_ptr_exception("Dereferencing NULL pointer * operator");
		return *ptr_;
	}
	catch (null_ptr_exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
	}
}

template <typename T>
T* smart_ptr<T>::operator->() { //arrow operator 
	try {
		if (ptr_ == nullptr)
			throw null_ptr_exception("Dereferencing NULL pointer -> operator");
		return ptr_;
	}
	catch (null_ptr_exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
	}
}


//--------------------------------Others-------------------------


template <typename T>
int smart_ptr<T>::ref_count() const
{
	try {
		if (ref_ == nullptr)
			throw null_ptr_exception("Dereferencing Ref_count is nullptr");
		return *ref_;
	}
	catch (null_ptr_exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 0;
	}
}

template <typename T>
bool smart_ptr<T>::clone()
{
	if (*ref_ == 1 || ptr_ == nullptr)
		return false;

	T* t_ptr = new T(*ptr_);
	this->delPtr();
	ptr_ = t_ptr;
	ref_ = new int(1);

	return true;
}




//---------------------------Helper functions--------------

template <typename T>
void smart_ptr<T>::delPtr()
{
	T* t_ptr = ptr_;
	int* t_ref = ref_;

	try {
		--*ref_;
		if (*ref_ == 0) {
			ptr_ = nullptr;
			ref_ = nullptr;
		}
	}
	catch (...) {
		std::cerr << "Destruction ERROR\n";
		++*ref_;
		ptr_ = t_ptr;
		ref_ = t_ref;
	}

	if (ptr_ == nullptr && ref_ == nullptr) {
		delete t_ptr;
		delete t_ref;
	}
}
