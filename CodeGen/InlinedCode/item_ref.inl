	// Reference to an element within the same package. Acts like a memory handle. 
	// It can be created, held and copied, but cannot be set to a specific value other than null.
	class item_ref
		{
		private:
			uuid id_m;

			// create from uuid, only allow this internally and from EntityReader
			static item_ref from_uuid( const uuid &id ) { item_ref val; val.id_m = id; return val; }
			friend class EntityReader;

		public:
			// ctors/dtor
			item_ref() noexcept : id_m( uuid_zero ) {}
			item_ref( const item_ref &other ) noexcept : id_m( other.id_m ) {}
			item_ref &operator=( const item_ref &other ) noexcept { this->id_m = other.id_m; return *this; }
			item_ref( item_ref &&other ) noexcept : id_m( other.id_m ) { other.id_m = uuid_zero; }
			item_ref &operator=( item_ref&& other ) noexcept { this->id_m = other.id_m; other.id_m = uuid_zero; return *this; }
			~item_ref() = default;

			// make a new reference with a unique uuid
			static item_ref make_ref();

			// comparing & sorting
			bool operator==( const item_ref &rval ) const { return this->id_m == rval.id_m; }
			bool operator!=( const item_ref &rval ) const { return this->id_m != rval.id_m; }
			bool operator<( const item_ref &rval ) const { return this->id_m < rval.id_m; }

			// conversions to boolean (behaves like a pointer, true if non-null) 
			operator bool() const { return this->id_m != uuid_zero; }
			operator uuid() const { return this->id_m; }

			// returns the "null" item_ref value 
			static const item_ref &null() { static item_ref null_id; return null_id; }
			
			//// inf and sup values, for comparing
			static item_ref &_inf() { static item_ref inf_id = from_uuid(uuid_inf); return inf_id; }
			static item_ref &_sup() { static item_ref sup_id = from_uuid(uuid_sup); return sup_id; }
		};

	inline std::ostream &operator<<( std::ostream &os, const item_ref &ref )
		{
		return os << ctle::value_to_hex_string((uuid)ref);
		}

	const item_ref item_ref_zero = item_ref::null();
	const item_ref item_ref_inf = item_ref::_inf();
	const item_ref item_ref_sup = item_ref::_sup();

