#include <cstdlib> // Add this line to include the appropriate header file

// define a list of quote Strings that can be used to print on the receipt
const char* quotes[3] = {
    "It's very attractive to the libertarian viewpoint if we can explain it properly. I'm better with code than with words though.- Satoshi Nakamoto, December 14, 2008",
    "In a few decades when the reward gets too small, the transaction fee will become the main compensation for nodes. - Satoshi Nakamoto, February 14, 2010",
    "I'm sure that in 20 years there will either be very large transaction volume or no volume. - Satoshi Nakamoto, February 14, 2010",
};

//  function that returns a random quote from the list of quotes
const char* getRandomQuote() {
    return quotes[random(0, 3)];
}