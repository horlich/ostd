
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



/************************ toUpper/toLower: **********************************/


template <typename charType>
std::basic_string<charType> convenience(const std::basic_string<charType>& ws, charType(*fp)(charType, const std::locale&))
{
   std::basic_string<charType> ret;
   static const std::locale loc("");
   for (charType wc : ws) {
      ret.push_back((*fp)(wc, loc));
   }
   return ret;
}


/************************* Tokenizer: ****************************************/

template <typename charType>
BStrVec<charType> tokenize_regex(const std::basic_string<charType>& str, const charType* muster) {
   BStrVec<charType> ret;
   std::basic_regex<charType> reg(muster);

   Xiterator<charType> anfang(str.begin(), str.end(), reg);
   for (Xiterator<charType> i = anfang; i != Xiterator<charType>(); ++i)
      ret.push_back(i->str());
   return ret;
}
