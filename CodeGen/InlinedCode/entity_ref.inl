	// Reference to another package. 
	class entity_ref
		{
		private:
			hash hash_m;

		public:
			// ctors/dtor
			entity_ref() noexcept : hash_m( hash_zero ) {}
			entity_ref( const hash &_hash ) noexcept : hash_m( _hash ) {}
			entity_ref( const entity_ref &other ) noexcept : hash_m( other.hash_m ) {}
			entity_ref &operator=( const entity_ref &other ) noexcept { this->hash_m = other.hash_m; return *this; }
			entity_ref( entity_ref &&other ) noexcept : hash_m( other.hash_m ) { other.hash_m = hash_zero; }
			entity_ref &operator=( entity_ref &&other ) noexcept { this->hash_m = other.hash_m; other.hash_m = hash_zero; return *this; }
			~entity_ref() = default;

			// comparing & sorting
			bool operator==( const entity_ref &rval ) const { return this->hash_m == rval.hash_m; }
			bool operator!=( const entity_ref &rval ) const { return this->hash_m != rval.hash_m; }
			bool operator<( const entity_ref &rval ) const { return this->hash_m < rval.hash_m; }

			// conversions to boolean (behaves like a pointer, true if non-null) 
			operator bool() const { return this->hash_m != hash_zero; }
			operator hash() const { return this->hash_m; }

			// returns the "null" entity_ref value 
			static const entity_ref &null() { static entity_ref null_id; return null_id; }
		};

	inline std::ostream &operator<<( std::ostream &os, const entity_ref &ref )
		{
		return os << hash( ref );
		}

	const entity_ref entity_ref_zero = entity_ref( hash_zero );
	const entity_ref entity_ref_inf = entity_ref( hash_inf );
	const entity_ref entity_ref_sup = entity_ref( hash_sup );

