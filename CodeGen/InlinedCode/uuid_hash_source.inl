
std::ostream &operator<<( std::ostream &os, const UUID &guid )
    {
    os << pds::value_to_hex_string( guid );
    return os;
    }

std::ostream &operator<<( std::ostream &os, const HASH &hsh )
    {
    os << pds::value_to_hex_string( hsh );
    return os;
    }
