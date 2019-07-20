#ifndef OLD_MAIN
#define NONIUS_RUNNER
#endif

#include "../Tests/nonius/nonius.h++"
#include "BoostAlgoString.h"
#include "BoostSpirit.h"
#include "Manual.h"
#include "X3.h"
#include <assert.h>
#include <chrono>

#ifdef OLD_MAIN
int main()
{
    std::string originalInput =
        "Five and Seven said nothing, but looked at Two. Two began in a low voice, 'Why the fact is, you see, Miss, this here ought to "
        "have been a red rose-tree, and we put a white one in by mistake; and if the Queen was to find it out, we should all have our "
        "heads cut off, you know. So you see, Miss, we're doing our best, afore she comes, to—' At this moment Five, who had been "
        "anxiously looking across the garden, called out 'The Queen! The Queen!' and the three gardeners instantly threw themselves flat "
        "upon their faces. There was a sound of many footsteps, and Alice looked round, eager to see the Queen.First came ten soldiers "
        "carrying clubs; these were all shaped like the three gardeners, oblong and flat, with their hands and feet at the corners: next "
        "the ten courtiers; these were ornamented all over with diamonds, and walked two and two, as the soldiers did. After these came "
        "the royal children; there were ten of them, and the little dears came jumping merrily along hand in hand, in couples: they were "
        "all ornamented with hearts. Next came the guests, mostly Kings and Queens, and among them Alice recognised the White Rabbit: it "
        "was talking in a hurried nervous manner, smiling at everything that was said, and went by without noticing her. Then followed the "
        "Knave of Hearts, carrying the King's crown on a crimson velvet cushion; and, last of all this grand procession, came THE KING AND "
        "QUEEN OF HEARTS.Alice was rather doubtful whether she ought not to lie down on her face like the three gardeners, but she could "
        "not remember ever having heard of such a rule at processions; 'and besides, what would be the use of a procession,' thought she, "
        "'if people had all to lie down upon their faces, so that they couldn't see it?' So she stood still where she was, and waited.When "
        "the procession came opposite to Alice, they all stopped and looked at her, and the Queen said severely 'Who is this?' She said it "
        "to the Knave of Hearts, who only bowed and smiled in reply.'Idiot!' said the Queen, tossing her head impatiently; and, turning to "
        "Alice, she went on, 'What's your name, child?''My name is Alice, so please your Majesty,' said Alice very politely; but she "
        "added, to herself, 'Why, they're only a pack of cards, after all. I needn't be afraid of them!''And who are these?' said the "
        "Queen, pointing to the three gardeners who were lying round the rosetree; for, you see, as they were lying on their faces, and "
        "the pattern on their backs was the same as the rest of the pack, she could not tell whether they were gardeners, or soldiers, or "
        "courtiers, or three of her own children.'How should I know?' said Alice, surprised at her own courage. 'It's no business of "
        "mine.'The Queen turned crimson with fury, and, after glaring at her for a moment like a wild beast, screamed 'Off with her head! "
        "Off—''Nonsense!' said Alice, very loudly and decidedly, and the Queen was silent.The King laid his hand upon her arm, and timidly "
        "said 'Consider, my dear: she is only a child!'The Queen turned angrily away from him, and said to the Knave 'Turn them over!'The "
        "Knave did so, very carefully, with one foot.'Get up!' said the Queen, in a shrill, loud voice, and the three gardeners instantly "
        "jumped up, and began bowing to the King, the Queen, the royal children, and everybody else.'Leave off that!' screamed the Queen. "
        "'You make me giddy.' And then, turning to the rose-tree, she went on, 'What have you been doing here?'";
    std::string inputWithtokens =
        "Five and Seven said nothing, but looked at $(Two). $(Two) began in a low voice, 'Why the fact is, you see, Miss, "
        "this here ought to have been a red rose-tree, and we put a white one in by mistake; and if the Queen was to "
        "find it out, we should all have our $(heads) cut off, you know. So you see, Miss, we're doing our best, afore "
        "she comes, to—' At this moment Five, who had been anxiously looking across the garden, called out 'The Queen! "
        "The Queen!' and the three gardeners instantly threw themselves flat upon their faces. There was a sound of "
        "many footsteps, and Alice looked round, eager to see the $(Queen).First came ten soldiers carrying clubs; these "
        "were all shaped like the three gardeners, oblong and flat, with their hands and feet at the corners: next the "
        "ten courtiers; these were ornamented all over with $(diamonds), and walked two and two, as the soldiers did. "
        "After these came the royal children; there were ten of them, and the little dears came jumping merrily along "
        "hand in hand, in couples: they were all ornamented with hearts. Next came the guests, mostly Kings and "
        "Queens, and among them Alice recognised the White Rabbit: it was talking in a hurried nervous manner, smiling "
        "at everything that was said, and went by without noticing her. Then followed the Knave of Hearts, carrying "
        "the King's crown on a crimson velvet cushion; and, last of all this grand procession, came THE KING AND QUEEN "
        "OF HEARTS.Alice was rather doubtful whether she ought not to lie down on her face like the three gardeners, "
        "but she could not remember ever having heard of such a rule at processions; 'and besides, what would be the "
        "use of a procession,' thought she, 'if people had all to lie down upon their faces, so that they couldn't see "
        "it?' So she stood still where she was, and waited.When the procession came opposite to Alice, they all "
        "stopped and looked at her, and the $(Queen) said severely 'Who is this?' She said it to the Knave of Hearts, who "
        "only bowed and smiled in reply.'Idiot!' said the Queen, tossing her head impatiently; and, turning to Alice, "
        "she went on, 'What's your name, child?''My name is Alice, so please your Majesty,' said Alice very politely; "
        "but she added, to herself, 'Why, they're only a pack of cards, after all. I needn't be afraid of them!''And "
        "who are these?' said the $(Queen), pointing to the three gardeners who were lying round the rosetree; for, "
        "you see, as they were lying on their faces, and the $(pattern) on their backs was the same as the rest of the "
        "pack, she could not tell whether they were gardeners, or soldiers, or courtiers, or three of her own children.'How "
        "should I know?' said Alice, surprised at her own courage. 'It's no business of mine.'The Queen turned crimson "
        "with fury, and, after glaring at her for a moment like a wild beast, screamed 'Off with her head! "
        "Off—''Nonsense!' said $(Alice), very loudly and decidedly, and the Queen was silent.The $(King) laid his hand "
        "upon her arm, and timidly said 'Consider, my dear: she is only a child!'The $(Queen) turned angrily away from him, "
        "and said to the $(Knave) 'Turn them over!'The $(Knave) did so, very carefully, with one foot.'Get up!' said "
        "the Queen, in a shrill, loud voice, and the three gardeners instantly jumped up, and began bowing to the King, "
        "the Queen, the royal children, and everybody else.'Leave off that!' screamed the Queen. 'You make me giddy.' "
        "And then, turning to the rose-tree, she went on, 'What have you been doing here?'";

    std::unordered_map<std::string, std::string> tokens{
        std::make_pair("$(Two)", "Two"),           std::make_pair("$(heads)", "heads"),
        std::make_pair("$(diamonds)", "diamonds"), std::make_pair("$(Queen)", "Queen"),
        std::make_pair("$(pattern)", "pattern"),   std::make_pair("$(Alice)", "Alice"),
        std::make_pair("$(King)", "King"),         std::make_pair("$(Knave)", "Knave"),
        std::make_pair("$(Why)", "Why"),           std::make_pair("$(glaring)", "glaring"),
        std::make_pair("$(name)", "name"),         std::make_pair("$(know)", "know"),
        std::make_pair("$(Idiot)", "Idiot"),       std::make_pair("$(children)", "children"),
        std::make_pair("$(Nonsense)", "Nonsense"), std::make_pair("$(procession)", "procession"),
    };

    size_t cycles = 10'000;

    {
        size_t count = 0;
        auto const symbols = [&] {
            bsq::symbols<char, std::string> symbols;
            for(const auto& token : tokens)
            {
                symbols.add(token.first.c_str(), token.second.c_str());
            }
            return symbols;
        }();
        auto start = std::chrono::high_resolution_clock::now();
        for(size_t i = 0; i < cycles; ++i)
        {
            std::string result = expandTokensBoostSpirit(inputWithtokens, symbols);
            assert(originalInput == result);
            count += result.size();
        }
        auto end = std::chrono::high_resolution_clock::now() - start;
        std::cout << "Boost.Spirit symbols result: " << count / cycles << "?=" << originalInput.size() << std::endl
                  << cycles << " cycles took " << std::chrono::duration_cast<std::chrono::milliseconds>(end).count() << "ms." << std::endl;
    }
    {
        size_t count = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for(size_t i = 0; i < cycles; ++i)
        {
            auto tmp = inputWithtokens;
            expandTokensBoostBoyerMoore(tmp, tokens);
            assert(tmp == originalInput);
            count += tmp.size();
        }
        auto end = std::chrono::high_resolution_clock::now() - start;
        std::cout << "Boyer-Moore results:" << count / cycles << "?=" << originalInput.size() << std::endl
                  << cycles << " cycles took " << std::chrono::duration_cast<std::chrono::milliseconds>(end).count() << "ms." << std::endl;
    }
    {
        size_t count = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for(size_t i = 0; i < cycles; ++i)
        {
            auto tmp = inputWithtokens;
            expandTokensBoyerMooreHorspool(tmp, tokens);
            assert(tmp == originalInput);
            count += tmp.size();
        }
        auto end = std::chrono::high_resolution_clock::now() - start;
        std::cout << "Boyer Moore Hospool result:" << count / cycles << "?=" << originalInput.size() << std::endl
                  << cycles << " cycles took " << std::chrono::duration_cast<std::chrono::milliseconds>(end).count() << "ms." << std::endl;
    }
    {
        size_t count = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for(size_t i = 0; i < cycles; ++i)
        {
            auto tmp = inputWithtokens;
            expandTokensBoostKnuthMorrisPratt(tmp, tokens);
            assert(tmp == originalInput);
            count += tmp.size();
        }
        auto end = std::chrono::high_resolution_clock::now() - start;
        std::cout << "Knuth Morris Pratt result: " << count / cycles << "?=" << originalInput.size() << std::endl
                  << cycles << " cycles took " << std::chrono::duration_cast<std::chrono::milliseconds>(end).count() << "ms." << std::endl;
    }
    {
        size_t count = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for(size_t i = 0; i < cycles; ++i)
        {
            auto tmp = inputWithtokens;
            expandTokensNaiveSTL(tmp, tokens);
            assert(tmp == originalInput);
            count += tmp.size();
        }
        auto end = std::chrono::high_resolution_clock::now() - start;
        std::cout << "Naive STL search and replace result: " << count / cycles << "?=" << originalInput.size() << std::endl
                  << cycles << " cycles took " << std::chrono::duration_cast<std::chrono::milliseconds>(end).count() << "ms." << std::endl;
    }
    {
        size_t count = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for(size_t i = 0; i < cycles; ++i)
        {
            std::string result = expandTokensBoostReplaceAll(inputWithtokens, tokens);
            assert(result == originalInput);
            count += result.size();
        }
        auto end = std::chrono::high_resolution_clock::now() - start;
        std::cout << "Boost replace_all result:" << count / cycles << "?=" << originalInput.size() << std::endl
                  << cycles << " cycles took " << std::chrono::duration_cast<std::chrono::milliseconds>(end).count() << "ms." << std::endl;
    }
    {
        size_t count = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for(size_t i = 0; i < cycles; ++i)
        {
            std::string result = expandTokensMFCMimicking(inputWithtokens, tokens);
            assert(result == originalInput);
            count += result.size();
        }
        auto end = std::chrono::high_resolution_clock::now() - start;
        std::cout << "Pseudo MFC replace_all result:" << count / cycles << "?=" << originalInput.size() << std::endl
                  << cycles << " cycles took " << std::chrono::duration_cast<std::chrono::milliseconds>(end).count() << "ms." << std::endl;
    }
    {
        size_t count = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for(size_t i = 0; i < cycles; ++i)
        {
            std::string result = expandTokensSehe(inputWithtokens, tokens);
            assert(result == originalInput);
            count += result.size();
        }
        auto end = std::chrono::high_resolution_clock::now() - start;
        std::cout << "Sehe manual result:" << count / cycles << "?=" << originalInput.size() << std::endl
                  << cycles << " cycles took " << std::chrono::duration_cast<std::chrono::milliseconds>(end).count() << "ms." << std::endl;
    }
    {
        size_t count = 0;
        auto const symbols = [&] {
            x3::symbols<char const*> symbols;
            for(const auto& token : tokens)
            {
                symbols.add(token.first.c_str(), token.second.c_str());
            }
            return symbols;
        }();
        auto start = std::chrono::high_resolution_clock::now();
        for(size_t i = 0; i < cycles; ++i)
        {
            std::string result = expandTokensX3(inputWithtokens, symbols);
            assert(result == originalInput);
            count += result.size();
        }
        auto end = std::chrono::high_resolution_clock::now() - start;
        std::cout << "Boost X3 result:" << count / cycles << "?=" << originalInput.size() << std::endl
                  << cycles << " cycles took " << std::chrono::duration_cast<std::chrono::milliseconds>(end).count() << "ms." << std::endl;
    }
    return 0;
}
#else

std::string originalInput =
    "Five and Seven said nothing, but looked at Two. Two began in a low voice, 'Why the fact is, you see, Miss, this here ought to "
    "have been a red rose-tree, and we put a white one in by mistake; and if the Queen was to find it out, we should all have our "
    "heads cut off, you know. So you see, Miss, we're doing our best, afore she comes, to—' At this moment Five, who had been "
    "anxiously looking across the garden, called out 'The Queen! The Queen!' and the three gardeners instantly threw themselves flat "
    "upon their faces. There was a sound of many footsteps, and Alice looked round, eager to see the Queen.First came ten soldiers "
    "carrying clubs; these were all shaped like the three gardeners, oblong and flat, with their hands and feet at the corners: next "
    "the ten courtiers; these were ornamented all over with diamonds, and walked two and two, as the soldiers did. After these came "
    "the royal children; there were ten of them, and the little dears came jumping merrily along hand in hand, in couples: they were "
    "all ornamented with hearts. Next came the guests, mostly Kings and Queens, and among them Alice recognised the White Rabbit: it "
    "was talking in a hurried nervous manner, smiling at everything that was said, and went by without noticing her. Then followed the "
    "Knave of Hearts, carrying the King's crown on a crimson velvet cushion; and, last of all this grand procession, came THE KING AND "
    "QUEEN OF HEARTS.Alice was rather doubtful whether she ought not to lie down on her face like the three gardeners, but she could "
    "not remember ever having heard of such a rule at processions; 'and besides, what would be the use of a procession,' thought she, "
    "'if people had all to lie down upon their faces, so that they couldn't see it?' So she stood still where she was, and waited.When "
    "the procession came opposite to Alice, they all stopped and looked at her, and the Queen said severely 'Who is this?' She said it "
    "to the Knave of Hearts, who only bowed and smiled in reply.'Idiot!' said the Queen, tossing her head impatiently; and, turning to "
    "Alice, she went on, 'What's your name, child?''My name is Alice, so please your Majesty,' said Alice very politely; but she "
    "added, to herself, 'Why, they're only a pack of cards, after all. I needn't be afraid of them!''And who are these?' said the "
    "Queen, pointing to the three gardeners who were lying round the rosetree; for, you see, as they were lying on their faces, and "
    "the pattern on their backs was the same as the rest of the pack, she could not tell whether they were gardeners, or soldiers, or "
    "courtiers, or three of her own children.'How should I know?' said Alice, surprised at her own courage. 'It's no business of "
    "mine.'The Queen turned crimson with fury, and, after glaring at her for a moment like a wild beast, screamed 'Off with her head! "
    "Off—''Nonsense!' said Alice, very loudly and decidedly, and the Queen was silent.The King laid his hand upon her arm, and timidly "
    "said 'Consider, my dear: she is only a child!'The Queen turned angrily away from him, and said to the Knave 'Turn them over!'The "
    "Knave did so, very carefully, with one foot.'Get up!' said the Queen, in a shrill, loud voice, and the three gardeners instantly "
    "jumped up, and began bowing to the King, the Queen, the royal children, and everybody else.'Leave off that!' screamed the Queen. "
    "'You make me giddy.' And then, turning to the rose-tree, she went on, 'What have you been doing here?'";
std::string inputWithtokens =
    "Five and Seven said nothing, but looked at $(Two). $(Two) began in a low voice, 'Why the fact is, you see, Miss, "
    "this here ought to have been a red rose-tree, and we put a white one in by mistake; and if the Queen was to "
    "find it out, we should all have our $(heads) cut off, you know. So you see, Miss, we're doing our best, afore "
    "she comes, to—' At this moment Five, who had been anxiously looking across the garden, called out 'The Queen! "
    "The Queen!' and the three gardeners instantly threw themselves flat upon their faces. There was a sound of "
    "many footsteps, and Alice looked round, eager to see the $(Queen).First came ten soldiers carrying clubs; these "
    "were all shaped like the three gardeners, oblong and flat, with their hands and feet at the corners: next the "
    "ten courtiers; these were ornamented all over with $(diamonds), and walked two and two, as the soldiers did. "
    "After these came the royal children; there were ten of them, and the little dears came jumping merrily along "
    "hand in hand, in couples: they were all ornamented with hearts. Next came the guests, mostly Kings and "
    "Queens, and among them Alice recognised the White Rabbit: it was talking in a hurried nervous manner, smiling "
    "at everything that was said, and went by without noticing her. Then followed the Knave of Hearts, carrying "
    "the King's crown on a crimson velvet cushion; and, last of all this grand procession, came THE KING AND QUEEN "
    "OF HEARTS.Alice was rather doubtful whether she ought not to lie down on her face like the three gardeners, "
    "but she could not remember ever having heard of such a rule at processions; 'and besides, what would be the "
    "use of a procession,' thought she, 'if people had all to lie down upon their faces, so that they couldn't see "
    "it?' So she stood still where she was, and waited.When the procession came opposite to Alice, they all "
    "stopped and looked at her, and the $(Queen) said severely 'Who is this?' She said it to the Knave of Hearts, who "
    "only bowed and smiled in reply.'Idiot!' said the Queen, tossing her head impatiently; and, turning to Alice, "
    "she went on, 'What's your name, child?''My name is Alice, so please your Majesty,' said Alice very politely; "
    "but she added, to herself, 'Why, they're only a pack of cards, after all. I needn't be afraid of them!''And "
    "who are these?' said the $(Queen), pointing to the three gardeners who were lying round the rosetree; for, "
    "you see, as they were lying on their faces, and the $(pattern) on their backs was the same as the rest of the "
    "pack, she could not tell whether they were gardeners, or soldiers, or courtiers, or three of her own children.'How "
    "should I know?' said Alice, surprised at her own courage. 'It's no business of mine.'The Queen turned crimson "
    "with fury, and, after glaring at her for a moment like a wild beast, screamed 'Off with her head! "
    "Off—''Nonsense!' said $(Alice), very loudly and decidedly, and the Queen was silent.The $(King) laid his hand "
    "upon her arm, and timidly said 'Consider, my dear: she is only a child!'The $(Queen) turned angrily away from him, "
    "and said to the $(Knave) 'Turn them over!'The $(Knave) did so, very carefully, with one foot.'Get up!' said "
    "the Queen, in a shrill, loud voice, and the three gardeners instantly jumped up, and began bowing to the King, "
    "the Queen, the royal children, and everybody else.'Leave off that!' screamed the Queen. 'You make me giddy.' "
    "And then, turning to the rose-tree, she went on, 'What have you been doing here?'";

std::unordered_map<std::string, std::string> tokens{
    std::make_pair("$(Two)", "Two"),           std::make_pair("$(heads)", "heads"),
    std::make_pair("$(diamonds)", "diamonds"), std::make_pair("$(Queen)", "Queen"),
    std::make_pair("$(pattern)", "pattern"),   std::make_pair("$(Alice)", "Alice"),
    std::make_pair("$(King)", "King"),         std::make_pair("$(Knave)", "Knave"),
    std::make_pair("$(Why)", "Why"),           std::make_pair("$(glaring)", "glaring"),
    std::make_pair("$(name)", "name"),         std::make_pair("$(know)", "know"),
    std::make_pair("$(Idiot)", "Idiot"),       std::make_pair("$(children)", "children"),
    std::make_pair("$(Nonsense)", "Nonsense"), std::make_pair("$(procession)", "procession"),
};

NONIUS_BENCHMARK("Naive STL", [](nonius::chronometer cm) {
    cm.measure([&](int) {
        std::string tmp = inputWithtokens;
        expandTokensNaiveSTL(tmp, tokens);
        assert(tmp == originalInput);
    });
})

NONIUS_BENCHMARK("MFCMimicking", [](nonius::chronometer cm) {
    const std::string tmp = inputWithtokens;

    std::string result;
    cm.measure([&](int) { result = expandTokensMFCMimicking(inputWithtokens, tokens); });
    assert(result == originalInput);
})

NONIUS_BENCHMARK("Sehe manual", [](nonius::chronometer cm) {
    const std::string tmp = inputWithtokens;

    std::string result;
    cm.measure([&](int) { result = expandTokensSehe(inputWithtokens, tokens); });
    assert(result == originalInput);
})

NONIUS_BENCHMARK("Boost replace_all", [](nonius::chronometer cm) {
    const std::string tmp = inputWithtokens;

    std::string result;
    cm.measure([&](int) { result = expandTokensBoostReplaceAll(inputWithtokens, tokens); });
    assert(result == originalInput);
})

NONIUS_BENCHMARK("Boost BM", [](nonius::chronometer cm) {
    cm.measure([&](int) {
        std::string tmp = inputWithtokens;
        expandTokensBoostBoyerMoore(tmp, tokens);
        assert(tmp == originalInput);
    });
})

NONIUS_BENCHMARK("Boost BMH", [](nonius::chronometer cm) {
    cm.measure([&](int) {
        std::string tmp = inputWithtokens;
        expandTokensBoyerMooreHorspool(tmp, tokens);
        assert(tmp == originalInput);
    });
})

NONIUS_BENCHMARK("Boost KMP", [](nonius::chronometer cm) {
    cm.measure([&](int) {
        std::string tmp = inputWithtokens;
        expandTokensBoostKnuthMorrisPratt(tmp, tokens);
        assert(tmp == originalInput);
    });
})

NONIUS_BENCHMARK("Boost QI", [](nonius::chronometer cm) {
    const std::string tmp = inputWithtokens;
    std::string result;
    auto const symbols = [&] {
        bsq::symbols<char, std::string> symbols;
        for(const auto& token : tokens)
        {
            symbols.add(token.first.c_str(), token.second.c_str());
        }
        return symbols;
    }();

    cm.measure([&](int) { result = expandTokensBoostSpirit(inputWithtokens, symbols); });
    assert(result == originalInput);
})

NONIUS_BENCHMARK("Boost X3 QI", [](nonius::chronometer cm) {
    const std::string tmp = inputWithtokens;
    std::string result;

    auto const symbols = [&] {
        x3::symbols<char const*> symbols;
        for(const auto& token : tokens)
        {
            symbols.add(token.first.c_str(), token.second.c_str());
        }
        return symbols;
    }();

    cm.measure([&](int) { result = expandTokensX3(inputWithtokens, symbols); });
    assert(result == originalInput);
})

NONIUS_BENCHMARK("Kreuzerkrieg manual", [](nonius::chronometer cm) {
    cm.measure([&](int) {
        std::string tmp = inputWithtokens;
        expandTokensKreuzerkrieg(tmp, tokens);
        assert(tmp == originalInput);
    });
})
#endif