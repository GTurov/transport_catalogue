#include "input_reader.h"

std::vector<std::string_view> SplitIntoWords(const std::string_view& text) {
    //std::cout<<"split "<<text<<std::endl;
    const std::string delimetrs = ":,->"s;
    std::vector<std::string_view> words;
    size_t word_begin = 0;
    size_t word_end = 0;
    while(word_begin != std::string::npos ) {
        word_begin = text.find_first_not_of(delimetrs, word_end);
        if (word_begin == std::string::npos) {
            break;
        }
        word_end = text.find_first_of(delimetrs,word_begin);
        std::string_view tmp = text.substr(word_begin,word_end-word_begin);
        size_t true_begin = tmp.find_first_not_of(" "s);
        size_t true_end = tmp.find_last_not_of(" "s);

        words.push_back(tmp.substr(true_begin, true_end-true_begin+1));
        //std::cout<<tmp.substr(true_begin, true_end-true_begin+1)<<std::endl;

    }
    return words;
}
