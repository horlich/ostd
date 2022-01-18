
/******************* CharArray: ************************/

template <size_t size>
void CharArray<size>::setStr(const char* str)
{
   std::strncpy(buf, str, size);
}

template <size_t size>
std::string CharArray<size>::getStr() const
{
   return buf;
}

template <size_t size>
size_t CharArray<size>::capacity() const
{
   return size;
}

