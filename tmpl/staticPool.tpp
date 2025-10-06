template <typename T, std::size_t N>
	requires( !std::is_polymorphic_v<T> )
StaticPool<T, N>::StaticPool() {
	for ( std::size_t i = 0; i < N; ++i ) {
		used[ i ] = false;
	}
}


template <typename T, std::size_t N>
	requires( !std::is_polymorphic_v<T> )
T *StaticPool<T, N>::allocate() noexcept {
	for ( std::size_t i = 0; i < N; ++i ) {
		if ( !used[ i ] ) {
			used[ i ] = true;
			return &pool[ i ];
		}
	}
	return nullptr;
}

template <typename T, std::size_t N>
	requires( !std::is_polymorphic_v<T> )
void StaticPool<T, N>::release( T *ptr ) noexcept {
	auto index = static_cast<std::size_t>( ptr - pool.data() );
	if ( index < N ) {
		used[ index ] = false;
	}
}

template <typename T, std::size_t N>
	requires( !std::is_polymorphic_v<T> )
std::size_t StaticPool<T, N>::usedCount() noexcept {
	return used.count();
}
