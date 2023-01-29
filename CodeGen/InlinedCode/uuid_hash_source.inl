
std::ostream &operator<<( std::ostream &os, const HASH &hsh )
    {
    os << pds::value_to_hex_string( hsh );
    return os;
    }
