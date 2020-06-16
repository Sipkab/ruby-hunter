template<typename T>
class has_public_member_@has_public_member_name@ {
public:
	template<typename C> static constexpr bool test(decltype(&C::@has_public_member_name@)){ return true; }
	template<typename C> static constexpr bool test(...) { return false; }
	static constexpr bool value = test<T>(nullptr);
};
