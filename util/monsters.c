#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

typedef int8_t  i8;
typedef uint8_t u8;

typedef int32_t  i32;
typedef uint32_t u32;

typedef i32 b32;

#include "json.c"

struct ActionSet {
    int count;

    char **names;
    char **descs;
    char **attack_bonuses;
    char **damage_dice;
    char **damage_bonuses;
};

b32 keyCmp(const char *key, jsmntok_t field, const char *json) {
    if (field.type != JSMN_STRING)
        return 0;

    int fieldLen = field.end - field.start;
    for (size_t i = 0; i < fieldLen; i += 1) {
        if (key[i] != json[field.start+i])
            return 0;
    }

    return 1;
}

void skipTokens(jsmntok_t *tokens, int *index) {
    switch (tokens[*index].type) {
    case JSMN_PRIMITIVE:
    case JSMN_STRING:
    case JSMN_UNDEFINED:
        *index = (*index)+1;
        break;

    case JSMN_ARRAY: {
        int start = *index;
        jsmntok_t arry = tokens[start++];

        for (size_t i = 0; i < arry.size; i++) {
            skipTokens(tokens, &start);
        }

        *index = start;
    } break;

    case JSMN_OBJECT: {
        int start = *index;
        jsmntok_t obj = tokens[start++];

        for (size_t i = 0; i < obj.size; i++) {
            skipTokens(tokens, &start); // key
            skipTokens(tokens, &start); // value
        }

        *index = start;
    } break;
    }
}

b32 extractString(
    const char *key,
    jsmntok_t field,
    jsmntok_t *child,
    const char *json,
    char **out
) {
    if (field.type != JSMN_STRING)
        return 0;

    int fieldLen = field.end - field.start;
    for (size_t i = 0; i < fieldLen; i += 1) {
        if (key[i] != json[field.start+i])
            return 0;
    }

    *out = strndup(json + child->start, child->end - child->start);
    return 1;
}

void ActionSetInit(struct ActionSet *actionSet, size_t count) {
    size_t size = count * sizeof(char *);
    actionSet->names = malloc(size);
    actionSet->descs = malloc(size);
    actionSet->attack_bonuses = malloc(size);
    actionSet->damage_dice = malloc(size);
    actionSet->damage_bonuses = malloc(size);
    actionSet->count = count;
}

b32 extractActionSet(
    const char *key,
    jsmntok_t field,
    jsmntok_t *child,
    const char *json,
    struct ActionSet *out
) {
    if (field.type != JSMN_STRING)
        return 0;

    int keyLen = field.end - field.start;
    for (size_t i = 0; i < keyLen; i += 1) {
        if (key[i] != json[field.start+i])
            return 0;
    }

    if (child->type != JSMN_ARRAY)
        return 0;

    int entryCount = child->size;
    ActionSetInit(out, entryCount);

    int offset = 1;

    for (size_t entry = 0; entry < entryCount; entry++) {
        jsmntok_t obj = child[offset++];

        int fieldCount = obj.size;

        char *desc = NULL;
        char *name = NULL;

        for (size_t i = 0; i < fieldCount; i += 1) {
            jsmntok_t key = child[offset++];
            jsmntok_t *token = &child[offset];

            extractString("desc", key, token, json, &out->descs[entry]);
            extractString("name", key, token, json, &out->names[entry]);

            skipTokens(child, &offset);
        }
    }

    return 1;
}

size_t ReadEntireFile(const char *path, char **data) {
    i32 fd = open(path, O_RDONLY);
    if (fd == -1) return 0;

    struct stat st;
    if (stat(path, &st) == -1) return 0;

    size_t len = st.st_size;
    size_t bufferLen = len + 1;
    char *address = malloc(bufferLen);

    i32 n;
    i32 bytesLeft = (i32)len;

    do {
        n = (i32)read(fd, address, bytesLeft);
        if (n == -1) break;
        bytesLeft -= n;
    } while (bytesLeft);

    close(fd);

    // If we failed while reading
    if (n == -1) {
        free(address);
        return 0;
    }

    address[len] = '\0';
    *data = address;
    return len;
}

struct Monsters {
	int count;

	char **names;
	char **sizes;
	char **types;
	char **subtypes;
	char **alignments;
	char **armor_class;
	char **hit_points;
	char **hit_dices;
	char **speeds;
	char **strengths;
	char **dexteritys;
	char **constitutions;
	char **intelligences;
	char **wisdoms;
	char **charismas;
	char **dexterity_saves;
	char **constitution_saves;
	char **wisdom_saves;
	char **charisma_saves;
	char **perceptions;
	char **stealths;
	char **damage_vulnerabilities;
	char **damage_resistances;
	char **damage_immunities;
	char **condition_immunities;
	char **senses;
	char **languages;
	char **challenge_ratings;

    struct ActionSet *actions;
    struct ActionSet *legendary_actions;
    struct ActionSet *special_abilities;
};

void MonstersInit(struct Monsters *monsters, int count) {
    int size = count * sizeof(char *);

    monsters->names = calloc(1, size);
    monsters->sizes = calloc(1, size);
    monsters->types = calloc(1, size);
    monsters->subtypes = calloc(1, size);
    monsters->alignments = calloc(1, size);
    monsters->armor_class = calloc(1, size);
    monsters->hit_points = calloc(1, size);
    monsters->hit_dices = calloc(1, size);
    monsters->speeds = calloc(1, size);
    monsters->strengths = calloc(1, size);
    monsters->dexteritys = calloc(1, size);
    monsters->constitutions = calloc(1, size);
    monsters->intelligences = calloc(1, size);
    monsters->wisdoms = calloc(1, size);
    monsters->charismas = calloc(1, size);
    monsters->dexterity_saves = calloc(1, size);
    monsters->constitution_saves = calloc(1, size);
    monsters->wisdom_saves = calloc(1, size);
    monsters->charisma_saves = calloc(1, size);
    monsters->perceptions = calloc(1, size);
    monsters->stealths = calloc(1, size);
    monsters->damage_vulnerabilities = calloc(1, size);
    monsters->damage_resistances = calloc(1, size);
    monsters->damage_immunities = calloc(1, size);
    monsters->condition_immunities = calloc(1, size);
    monsters->senses = calloc(1, size);
    monsters->languages = calloc(1, size);
    monsters->challenge_ratings = calloc(1, size);

    int actionSize = count * sizeof(struct ActionSet);
    monsters->actions = calloc(1, actionSize);
    monsters->legendary_actions = calloc(1, actionSize);
    monsters->special_abilities = calloc(1, actionSize);

    monsters->count = count;
}

b32 ParseMonsters(char *data, size_t len, struct Monsters *monstersOut) {
    jsmn_parser parser;
    jsmn_init(&parser);

    int tokenCount = jsmn_parse(&parser, data, len, NULL, 0);
    if (tokenCount < 1) {
        printf("Failed to parse json: %d\n", tokenCount);
        return false;
    }

    jsmntok_t *tokens = calloc(tokenCount, sizeof(jsmntok_t));
    if (!tokens) {
        return false;
    }

    printf("There are %d json tokens\n", tokenCount);

    jsmn_init(&parser);

    tokenCount = jsmn_parse(&parser, data, len, tokens, tokenCount);
    if (tokenCount < 0) {
        printf("Failed to load tokens: %d\n", tokenCount);
        return false;
    }

    if (tokens[0].type != JSMN_ARRAY) {
        return false;
    }

    int monsterCount = tokens[0].size;

    printf("There are %d monsters\n", monsterCount);

    int offset = 1;

    struct Monsters monsters;
    MonstersInit(&monsters, monsterCount);

    for (int monster = 0; monster < monsterCount; monster += 1) {
        jsmntok_t obj = tokens[offset++];

        if (obj.type != JSMN_OBJECT) {
            return false;
        }

        int fields = obj.size;

        for (size_t i = 0; i < fields; i += 1) {
            jsmntok_t key = tokens[offset++];
            jsmntok_t *token = &tokens[offset];

            extractString("name", key, token, data, &monsters.names[monster]);
            extractString("size", key, token, data, &monsters.sizes[monster]);
            extractString("type", key, token, data, &monsters.types[monster]);
            extractString("subtype", key, token, data, &monsters.subtypes[monster]);
            extractString("alignment", key, token, data, &monsters.alignments[monster]);
            extractString("armor_class", key, token, data, &monsters.armor_class[monster]);
            extractString("hit_points", key, token, data, &monsters.hit_points[monster]);
            extractString("hit_dice", key, token, data, &monsters.hit_dices[monster]);
            extractString("speed", key, token, data, &monsters.speeds[monster]);
            extractString("strength", key, token, data, &monsters.strengths[monster]);
            extractString("dexterity", key, token, data, &monsters.dexteritys[monster]);
            extractString("constitution", key, token, data, &monsters.constitutions[monster]);
            extractString("intelligence", key, token, data, &monsters.intelligences[monster]);
            extractString("wisdom", key, token, data, &monsters.wisdoms[monster]);
            extractString("charisma", key, token, data, &monsters.charismas[monster]);
            extractString("dexterity_save", key, token, data, &monsters.dexterity_saves[monster]);
            extractString("constitution_save", key, token, data, &monsters.constitution_saves[monster]);
            extractString("wisdom_save", key, token, data, &monsters.wisdom_saves[monster]);
            extractString("charisma_save", key, token, data, &monsters.charisma_saves[monster]);
            extractString("perception", key, token, data, &monsters.perceptions[monster]);
            extractString("stealth", key, token, data, &monsters.stealths[monster]);
            extractString("damage_vulnerabilities", key, token, data, &monsters.damage_vulnerabilities[monster]);
            extractString("damage_resistances", key, token, data, &monsters.damage_resistances[monster]);
            extractString("damage_immunities", key, token, data, &monsters.damage_immunities[monster]);
            extractString("condition_immunities", key, token, data, &monsters.condition_immunities[monster]);
            extractString("senses", key, token, data, &monsters.senses[monster]);
            extractString("languages", key, token, data, &monsters.languages[monster]);
            extractString("challenge_rating", key, token, data, &monsters.challenge_ratings[monster]);

            extractActionSet("actions", key, token, data, &monsters.actions[monster]);
            extractActionSet("special_abilities", key, token, data, &monsters.special_abilities[monster]);
            extractActionSet("legendary_actions", key, token, data, &monsters.legendary_actions[monster]);
            skipTokens(tokens, &offset);
        }

        printf("  ----%s----\n\n", monsters.names[monster]);
        if (monsters.actions[monster].count) {
            printf("  actions:\n");
            for (size_t i = 0; i < monsters.actions[monster].count; i += 1) {
                printf("    - %s\n", monsters.actions[monster].names[i]);
            }
            printf("\n");
        }

        if (monsters.legendary_actions[monster].count) {
            printf("  legendary actions:\n");
            for (size_t i = 0; i < monsters.legendary_actions[monster].count; i += 1) {
                printf("    - %s\n", monsters.legendary_actions[monster].names[i]);
            }
            printf("\n");
        }

        if (monsters.special_abilities[monster].count) {
            printf("  special abilities:\n");
            for (size_t i = 0; i < monsters.special_abilities[monster].count; i += 1) {
                printf("    - %s\n", monsters.special_abilities[monster].names[i]);
            }
            printf("\n");
        }
    }

    *monstersOut = monsters;
    return true;
}

void CodegenMonsters(FILE *file, struct Monsters *monsters) {
    int monsterCount = monsters->count;

    fprintf(file, "const int MonsterCount = %d;\n", monsterCount);
    fprintf(file, "\n");
    fprintf(file, "struct Action {\n    char *name;\n    char *description;\n    char *attack_bonus;\n    char *damage_dice;\n    char *damage_bonus;\n};\n");

    struct {
        char *name;
        char **values;
    } members[] = {
        { "names", monsters->names },
        { "sizes", monsters->sizes },
        { "types", monsters->types },
        { "subtypes", monsters->subtypes },
        { "alignments", monsters->alignments },
        { "armor_class", monsters->armor_class },
        { "hit_points", monsters->hit_points },
        { "hit_dices", monsters->hit_dices },
        { "speeds", monsters->speeds },
        { "strengths", monsters->strengths },
        { "dexteritys", monsters->dexteritys },
        { "constitutions", monsters->constitutions },
        { "intelligences", monsters->intelligences },
        { "wisdoms", monsters->wisdoms },
        { "charismas", monsters->charismas },
        { "dexterity_saves", monsters->dexterity_saves },
        { "constitution_saves", monsters->constitution_saves },
        { "wisdom_saves", monsters->wisdom_saves },
        { "charisma_saves", monsters->charisma_saves },
        { "perceptions", monsters->perceptions },
        { "stealths", monsters->stealths },
        { "damage_vulnerabilities", monsters->damage_vulnerabilities },
        { "damage_resistances", monsters->damage_resistances },
        { "damage_immunities", monsters->damage_immunities },
        { "condition_immunities", monsters->condition_immunities },
        { "senses", monsters->senses },
        { "languages", monsters->languages },
        { "challenge_ratings", monsters->challenge_ratings }    
    };

    for (int i = 0; i < sizeof(members)/sizeof(members[0]); i += 1) {
        fprintf(file, "\nchar *%s[%d] = {\n", members[i].name, monsterCount);

        char **values = members[i].values;
        for (int j = 0; j < monsterCount; j += 1) {
            fprintf(file, "    \"%s\",\n", values[j] ?: "");
        }

        fprintf(file, "};\n");
    }
}

int main(int argc, char **argv) {
    char *path = "data/monsters.json";

    if (argc > 1)
        path = argv[1];

    char *data = NULL;
    size_t len = ReadEntireFile(path, &data);

    if (!data || !len) {
        fprintf(stderr, "Unable to read '%s'\n", path);
        return 1;
    }

    printf("Read %lu bytes\n", len);

    b32 ok;
    struct Monsters monsters;

    ok = ParseMonsters(data, len, &monsters);
    if (!ok) {
        fprintf(stderr, "Ran into an error while parsing monsters\n");
        return 1;
    }

    FILE *file = fopen("src/monsters.c", "w");
    CodegenMonsters(file, &monsters);

    return 0;
}
