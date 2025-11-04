#include <ctype.h>
#include <stdbool.h>                              // token_ends_a_sentence() bruger bool, så denne skal inkluderes.
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_WORD_COUNT 15000                      // 15'000 virker ikke i ældre C-standarder, så jeg har ændret dette for en sikkerheds skyld.
#define MAX_SUCCESSOR_COUNT MAX_WORD_COUNT / 2

char book[] = {
#embed "pg84.txt" /// Stores the content of the file as an array of chars.
    , '\0'};      /// Makes `book` a string.

/// Array of tokens registered so far.
/// No duplicates are allowed.
char *tokens[MAX_WORD_COUNT];
/// `tokens`'s current size
size_t tokens_size = 0;

/// Array of successor tokens
/// One token can have many successor tokens. `succs[x]` corresponds to
/// `token[x]`'s successors.
/// We store directly tokens instead of token_ids, because we will directly
/// print them. If we wanted to delete the book, then it would make more sense
/// to store `token_id`s
char *succs[MAX_WORD_COUNT][MAX_SUCCESSOR_COUNT];
/// `succs`'s current size
size_t succs_sizes[MAX_WORD_COUNT];

/// Overwrites non-printable characters in `book` with a space.
/// Non-printable characters may lead to duplicates like
/// `"\xefthe" and "the"` even both print `the`.
void replace_non_printable_chars_with_space() {
  // YOUR CODE HERE
  for (char *p = book; *p != '\0'; ++p) {                       // Går alle tegn i 'book' igennem,
        if (!isprint((unsigned char)*p)) *p = ' ';              // Erstatter ikke-printbare tegn med mellemrum.
    }
}

/// Returns the id (index) of the token, creating it if necessary.
///
/// Returns token id if token exists in \c tokens, otherwise creates a new entry
/// in \c tokens and returns its token id.
///
/// \param token token to look up (or insert)
/// \return Index of `token` in \c tokens array
size_t token_id(char *token) {
  size_t id;
  for (id = 0; id < tokens_size; ++id) {
    if (strcmp(tokens[id], token) == 0) {
      return id;
    }
  }
  tokens[id] = token;
  ++tokens_size;
  return id;
}

/// Appends the token \c succ to the successors list of \c token.
void append_to_succs(char *token, char *succ) {
  auto next_empty_index_ptr = &succs_sizes[token_id(token)];

  if (*next_empty_index_ptr >= MAX_SUCCESSOR_COUNT) {
    printf("Successor array full.");
    exit(EXIT_FAILURE);
  }

  succs[token_id(token)][(*next_empty_index_ptr)++] = succ;
}

/// Creates tokens on \c book and fills \c tokens and \c succs using
/// the functions \c token_id and \c append_to_succs.
void tokenize_and_fill_succs(char *delimiters, char *str) {
  // YOUR CODE HERE
    memset(succs_sizes, 0, sizeof succs_sizes);                     // Nulstiller succ-sizes (init med mem*) .
    tokens_size = 0;                                                // Start forfra på token-tabellen.

    char *forrige = strtok(str, delimiters);                       // Tager første token (splitter in-place i 'book')
    if (!forrige) return;                                          // Er teksten tom? Intet at gøre.
    token_id(forrige);                                             // Registrér første token i tabellen.

    for (char *naeste = strtok(NULL, delimiters);                  // Iterérer over efterfølgende tokens.
         naeste != NULL;
         naeste = strtok(NULL, delimiters)) {
        append_to_succs(forrige, naeste);                          // registrér relation: forrige -> næste
        token_id(naeste);                                          // Er 'naeste' token registreret?
        forrige = naeste;                                          // Flytter 'vinduet' til næste token.
    }
}

/// Returns last character of a string
char last_char(char *str) {
  // YOUR CODE HERE
  size_t n = strlen(str);                                          // Strengens længde.
    while (n > 0 && isspace((unsigned char)str[n-1]))              // Mellemrum til sidst? Skip det.
        --n;
    return (n == 0) ? '\0' : str[n-1];                             // Returnerer det sidste “synlige” tegn eller '\0'
}

/// Returns whether the token ends with `!`, `?` or `.`.
bool token_ends_a_sentence(char *token) {
  // YOUR CODE HERE
  char c = last_char(token);                                       // Hvad er det sidste relevante tegn i token?
    return (c == '.') || (c == '!') || (c == '?');                 // Sætning slutter på ., ! eller ?
}

/// Returns a random `token_id` that corresponds to a `token` that starts with a
/// capital letter.
/// Uses \c tokens and \c tokens_size.
size_t random_token_id_that_starts_a_sentence() {
  // YOUR CODE HERE
    size_t kandidat_ids[1024];                                     // Buffer til kandidater med stort begyndelsesbogstav.
    size_t k_sz = 0;                                               // aktuel kandidatstørrelse
    for (size_t i = 0; i < tokens_size && k_sz < 1024; ++i) {
        unsigned char f = (unsigned char)tokens[i][0];             // Første tegn i token.
        if (isupper(f)) kandidat_ids[k_sz++] = i;                  // Stort bogstav? => kandidat.
    }
    if (k_sz == 0) return 0;                                       // Er der ingen kandidater? Returnér 0.
    return kandidat_ids[rand() % k_sz];                            // Vælger tilfældig start-token.
}

/// Generates a random sentence using \c tokens, \c succs, and \c succs_sizes.
/// The sentence array will be filled up to \c sentence_size-1 characters using
/// random tokens until:
/// - a token is found where \c token_ends_a_sentence
/// - or more tokens cannot be concatenated to the \c sentence anymore.
/// Returns the filled sentence array.
///
/// @param sentence array what will be used for the sentence.
//
//                  Will be overwritten. Does not have to be initialized.
/// @param sentence_size
/// @return input sentence pointer
char *generate_sentence(char *sentence, size_t sentence_size) {
  size_t current_token_id = random_token_id_that_starts_a_sentence();
  auto token = tokens[current_token_id];

  sentence[0] = '\0';
  strcat(sentence, token);
  if (token_ends_a_sentence(token))
    return sentence;

  // Calculated sentence length for the next iteration.
  // Used to stop the loop if the length exceeds sentence size
  size_t sentence_len_next;
  // Concatenates random successors to the sentence as long as
  // `sentence` can hold them.
  do {
    // YOUR CODE HERE
        size_t succ_sz = succs_sizes[current_token_id];           // Antallet af  efterfølgere for nuværende token.
        if (succ_sz == 0) break;                                  // Er der ingen efterfølgere? => stop.

        char *naeste = succs[current_token_id][rand() % succ_sz]; // Vælger en tilfældig efterfølger.

        // Beregner om der er plads til " mellemrum + naeste ",
        size_t len_sentence = strlen(sentence);
        size_t len_naeste   = strlen(naeste);
        sentence_len_next   = len_sentence + 1 + len_naeste;      // Læg +1 til størrelsen for mellemrum.

        if (sentence_len_next >= sentence_size) break;            // Overstiger ny sætning sentence_size? => stop.

        sentence[len_sentence] = ' ';
        sentence[len_sentence + 1] = '\0';                        // Nulterminerer sentence.
        strcat(sentence, naeste);                                 // Tilføjer næste token.

        current_token_id = token_id(naeste);

        if (token_ends_a_sentence(naeste)) break;                 // Er token et sluttegn? => afslut sætning.
  } while (sentence_len_next < sentence_size - 1);
  return sentence;
}

int main() {
  replace_non_printable_chars_with_space();

  char *delimiters = " \n\r";
  tokenize_and_fill_succs(delimiters, book);

  char sentence[1000];
  srand(time(nullptr)); // Be random each time we run the program

  // Generate sentences until we find a question sentence.
  do {
    generate_sentence(sentence, sizeof sentence);
  } while (last_char(sentence) != '?');
  puts(sentence);
  puts("");

  // Initialize `sentence` and then generate sentences until we find a sentence
  // ending with an exclamation mark.
  do {
    generate_sentence(sentence, sizeof sentence);
  } while (last_char(sentence) != '!');
  puts(sentence);
}