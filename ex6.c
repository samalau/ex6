/******************
Name: Samantha Newmark
ID: ---------
Assignment: ex6
*******************/

#include "ex6.h"

static const PokemonData *findPokemonByID(int id) {
    if (id < 1 || id > (int)(sizeof(pokedex) / sizeof(pokedex[0]))) {
		printf("Invalid Pokemon ID.\n");
        return NULL;
    }

    PokemonNode *root = getRoot();
    if (root) {
        PokemonNode *foundNode = searchPokemonBFS(root, id);
        if (foundNode) {
            printf("Pokemon with ID %d is already in the Pokedex. No changes made.\n", id);
            return NULL;
        }
    }
    return &pokedex[id - 1];
}

PokemonNode *getRoot() {
    if (!currentOwner) {
        return NULL;
    }
    return currentOwner->pokedexRoot;
}


void trimWhitespace(char *str) {
	if (str == NULL) return;
	char *start = str;
	while (isspace((unsigned char)*start)) {
		start++;
	}
	if (*start == '\0') {
		*str = '\0';
		return;
	}
	char *end = start + strlen(start) - 1;
	while (end > start && isspace((unsigned char)*end)) {
		end--;
	}
	*(end + 1) = '\0';
	if (start != str) {
		// length of trimmed substring
		size_t length = end - start + 1;
		// copy substring plus null terminator
		memmove(str, start, length + 1);
	}
	return;
}

char *myStrdup(const char *src) {
	if (!src) {
		return NULL;
	}
	size_t len = strlen(src) + 1;
	char *copy = (char *)malloc(len);
	if (!copy) {
		return NULL;
	}
	memcpy(copy, src, len);
	return copy;
}


char *getlineCustom(char **buffer, size_t *size) {
    if (buffer == NULL || size == NULL) {
        printf("Invalid option\n");
        return NULL;
    }

    if (*buffer) {
        free(*buffer);
        *buffer = NULL;
    }

    *size = INITIAL_BUFFER_SIZE;
    *buffer = malloc(*size);
    if (*buffer == NULL) {
		stop();
		freeAllOwners();
		printf("Goodbye!\n");
		exit(EXIT_FAILURE);
    }

    size_t len = 0;

    while (1) {
        if (fgets(*buffer + len, *size - len, stdin) == NULL) {
            if (*buffer) {
				free(*buffer);
				*buffer = NULL;
			}
			stop();
			freeAllOwners();
			printf("Goodbye!\n");
			exit(EXIT_FAILURE);
        }

        len += strlen(*buffer + len);

        if (len > 0 && (*buffer)[len - 1] == '\n') {
            (*buffer)[len - 1] = '\0';
            break;
        }

        // expand buffer if full
        if (len == *size - 1) {
            *size *= EXPANSION_FACTOR;
            char *temp = realloc(*buffer, *size);
            if (temp == NULL) {
                if (*buffer) {
					free(*buffer);
                	*buffer = NULL;
				}
				stop();
				freeAllOwners();
				printf("Goodbye!\n");
				exit(EXIT_FAILURE);
            }
            *buffer = temp;
        }
    }
    return *buffer;
}


char *read() {
	char *buffer = NULL;
	size_t size = 0;
	if (getlineCustom(&buffer, &size) == NULL || buffer[0] == '\0') {
		if (buffer) {
			free(buffer);
			buffer = NULL;
		}
		return NULL;
	}
	return buffer;
}

char *input() {
	char *buffer = read();
	if (buffer == NULL) {
		return NULL;
	}
	return buffer;
}

char *strtok_r_impl(char *str, const char *delim, char **saveptr) {
	if (str == NULL) {
		str = *saveptr;
	}
	if (str == NULL) {
		return NULL;
	}
	str += strspn(str, delim);
	if (*str == '\0') {
		*saveptr = NULL;
		return NULL;
	}
	char *end = str + strcspn(str, delim);
	if (*end == '\0') {
		*saveptr = NULL;
	} else {
		*end = '\0';
		*saveptr = end + 1;
	}
	return str;
}

Classification classify(const char *input) {
	Classification result = {.isType = IS_STRING, .value.strValue = NULL};

	if (!input || *input == '\0') {
		result.value.strValue = myStrdup("");
		return result;
	}

	trimWhitespace((char *)input);
	if (*input == '\0') {
		result.value.strValue = myStrdup("");
		return result;
	}

	char *endptr = NULL;
	long value = strtol(input, &endptr, 10);
	if (*endptr == '\0' && value >= MIN_INT && value <= MAX_INT) {
		result.isType = IS_INTEGER;
		result.value.intValue = (int)value;
	} else {
		result.value.strValue = myStrdup(input);
	}

	return result;
}

void freeClassification(Classification *result) {
	if (result && !result->isType && result->value.strValue) {
		free(result->value.strValue);
		result->value.strValue = NULL;
	}
	return;
}

void *getDynamicInput() {
	char *userInput = input();
	if (!userInput) {
		return NULL;
	}

	trimWhitespace(userInput);

    if (*userInput == '\0') {
        free(userInput);
		userInput = NULL;
        return NULL;
    }
	return userInput;
}

int readIntSafe() {
	char *input = (char *)getDynamicInput();
	if (!input) {
		return NONE;
	}

	Classification temp = classify(input);
	if (input) {
		free(input);
		input = NULL;
	}
	if (temp.isType == IS_INTEGER && temp.value.intValue >= 0) {
		return temp.value.intValue;
	}
	return NONE;
}


void *menuNavigator(MenuIndex menuIndex, void *param) {
	int choice = 0, fullMenu = YES;
	const Menu *currentMenu = NULL;
	Menu ownerMenu = {NULL, NULL, 0, NONE};

	do {
		if (menuIndex == MAIN_MENU && choice == MAIN_MENU_ITEM_COUNT) {
			cleanupResources(&param, &ownerMenu, &keepGoing);
			printf("Goodbye!\n");
			exit(EXIT_SUCCESS);
		}

		if (fullMenu) {
			if (menuIndex == EXISTING_POKEDEX_MENU && currentOwner && ownerHead) {
				printf("\n-- %s's Pokedex Menu --\n", currentOwner->ownerName);
				ownerMenu = generateOwnerMenu(ownerHead);
				currentMenu = &ownerMenu;
			}
			currentMenu = &menus[menuIndex];
			printMenu(currentMenu);
		}

		printf("Your choice: ");
		choice = readIntSafe();

		if (!keepGoing) {
			menuIndex = MAIN_MENU;
			cleanupResources(&param, &ownerMenu, &keepGoing);
			printf("Goodbye!\n");
			exit(EXIT_SUCCESS);
		}

		if (!currentMenu || choice < 1 || choice > currentMenu->itemCount) {
			fullMenu = NO;
			printf("Invalid choice.\n\n");
			continue;
		}

		if (menuIndex == MAIN_MENU) {
			currentOwner = NULL;
		}

		const MenuItem *selectedItem = &currentMenu->items[choice - 1];

		if (menuIndex == EXISTING_POKEDEX_MENU && (choice == currentMenu->itemCount || currentOwner == NULL)) {
			menuIndex = MAIN_MENU;
			cleanupResources(&param, &ownerMenu, &keepGoing);
			printf("Back to Main Menu.\n\n");
			return menuNavigator(MAIN_MENU, NULL);
		}

		if (menuIndex == DISPLAY_MENU && selectedItem->MenuAction) {
			selectedItem->MenuAction(menuIndex, choice, param);
			cleanupResources(&param, &ownerMenu, &keepGoing);
			return menuNavigator(EXISTING_POKEDEX_MENU, NULL);
		}

		if (selectedItem->MenuAction) {
			invokeMenuAction((GenericMenuAction)(selectedItem->MenuAction), menuIndex, choice, param);
		}

		if (selectedItem->nextMenuIndex != (MenuIndex)-1) {
			menuIndex = selectedItem->nextMenuIndex;
		}

		fullMenu = YES;
	} while (keepGoing);

	cleanupResources(&param, &ownerMenu, &keepGoing);
	printf("Goodbye!\n");
	exit(EXIT_SUCCESS);
}

// void *menuNavigator(MenuIndex menuIndex, void *param) {
// 	int choice = 0, fullMenu = YES;
// 	const Menu *currentMenu = NULL;
//     Menu ownerMenu = {NULL, NULL, 0, NONE};

// 	do {
// 		if (menuIndex == MAIN_MENU && choice == MAIN_MENU_ITEM_COUNT) {
// 			if (param) {
// 				free(param);
// 				param = NULL;
// 			}
// 			if (ownerMenu.items) {
// 				free((void *)ownerMenu.items);
// 				ownerMenu.items = NULL;
// 			}
// 			stop();
// 			freeAllOwners();
// 			printf("Goodbye!\n");
// 			exit(EXIT_SUCCESS);
// 		}
// 		if (fullMenu) {
// 			if (menuIndex == EXISTING_POKEDEX_MENU && currentOwner && ownerHead) {
// 				printf("\n-- %s's Pokedex Menu --\n", currentOwner->ownerName);
// 				ownerMenu = generateOwnerMenu(ownerHead);
// 				currentMenu = &ownerMenu;
// 			}
// 			currentMenu = &menus[menuIndex];
// 			printMenu(currentMenu);
// 		}

// 		printf("Your choice: ");
// 		choice = readIntSafe();

// 		if (!keepGoing) {
// 			menuIndex = MAIN_MENU;
// 			if (param) {
// 				free(param);
// 				param = NULL;
// 			}
// 			if (ownerMenu.items) {
// 				free((void *)ownerMenu.items);
// 				ownerMenu.items = NULL;
// 			}
// 			freeAllOwners();
// 			printf("Goodbye!\n");
// 			exit(EXIT_SUCCESS);
// 		}

// 		if (!currentMenu || choice < 1 || choice > currentMenu->itemCount) {
// 			fullMenu = NO;
// 			printf("Invalid choice.\n\n");
// 			continue;
// 		}

// 		if (menuIndex == MAIN_MENU) {
// 			currentOwner = NULL;
// 		}

// 		if (menuIndex == MAIN_MENU && choice == MAIN_MENU_ITEM_COUNT) {
// 			if (menuIndex == MAIN_MENU && choice == MAIN_MENU_ITEM_COUNT) {
// 				if (param) {
// 					free(param);
// 					param = NULL;
// 				}
// 				if (ownerMenu.items) {
// 					free((void *)ownerMenu.items);
// 					ownerMenu.items = NULL;
// 				}
// 				stop();
// 				freeAllOwners();
// 				printf("Goodbye!\n");
// 				exit(EXIT_SUCCESS);
// 			}
// 		}

// 		const MenuItem *selectedItem = &currentMenu->items[choice - 1];
// 		if (menuIndex == EXISTING_POKEDEX_MENU && (choice == currentMenu->itemCount || currentOwner == NULL)) {
// 			menuIndex = MAIN_MENU;
			
// 			if (ownerMenu.items) {
// 				free((void *)ownerMenu.items);
// 				ownerMenu.items = NULL;
// 			}
// 			printf("Back to Main Menu.\n\n");
// 			if (param) {
// 				free(param);
// 				param = NULL;
// 			}
// 			fullMenu = YES;
// 			return menuNavigator(MAIN_MENU, NULL);
// 		}

// 		if (menuIndex == DISPLAY_MENU && selectedItem->MenuAction) {
// 			selectedItem->MenuAction(menuIndex, choice, param);
// 			fullMenu = YES;
// 			if (param) {
// 				free(param);
// 				param = NULL;
// 			}
// 			return menuNavigator(EXISTING_POKEDEX_MENU, NULL);
// 		}

// 		if (selectedItem->MenuAction) {
// 			fullMenu = YES;
// 			invokeMenuAction((GenericMenuAction)(selectedItem->MenuAction), menuIndex, choice, param);
// 		}

// 		if (selectedItem->nextMenuIndex != (MenuIndex)-1) {
// 			fullMenu = YES;
// 			menuIndex = selectedItem->nextMenuIndex;
// 		}
// 	} while (keepGoing);

// 	if (menuIndex == MAIN_MENU && choice == MAIN_MENU_ITEM_COUNT) {
// 		if (param) {
// 			free(param);
// 			param = NULL;
// 		}
// 		if (ownerMenu.items) {
// 			free((void *)ownerMenu.items);
// 			ownerMenu.items = NULL;
// 		}
// 		stop();
// 		freeAllOwners();
// 		printf("Goodbye!\n");
// 		exit(EXIT_SUCCESS);
// 	}
// 	return NULL;
// }

void *invokeMenuAction(GenericMenuAction action, MenuIndex menuIndex, int choice, void *param) {
	if (action) {
		return action(menuIndex, choice, param);
	}
	return NULL;
}

void printMenu(const Menu *menu) {
	if (!menu) {
		return;
	}

	if (!menu->items || menu->itemCount <= 0) {
		return;
	}

	const char *titleStyle;
	switch (menu->styleType) {
		case BORDERED_STYLE:
			titleStyle = "\n=== %s ===\n";
			break;
		case UNBORDERED_STYLE:
			titleStyle = "\n%s:\n";
			break;
		default:
			titleStyle = "%s:\n";
	}
	if (menu->title) {
		printf(titleStyle, menu->title);
	}

	for (int i = 0; i < menu->itemCount; i++) {
		if (!menu->items[i].prompt) {
			continue;
		}
		printf("%d. %s\n", i + 1, menu->items[i].prompt);
	}
	return;
}

Menu generateOwnerMenu(OwnerNode *selectedOwner) {
	NodeArray pokedexNodes = {NULL, 0, 10};
	pokedexNodes.nodes = malloc(pokedexNodes.capacity * sizeof(PokemonNode *));
	if (!pokedexNodes.nodes) {
		stop();
		freeAllOwners();
		printf("Goodbye!\n");
		exit(EXIT_FAILURE);
	}

	collectAll(selectedOwner->pokedexRoot, &pokedexNodes);

	MenuItem *menuItems = malloc(pokedexNodes.size * sizeof(MenuItem));
	if (!menuItems) {
		if (pokedexNodes.nodes) {
			free(pokedexNodes.nodes);
			pokedexNodes.nodes = NULL;
		}
		stop();
		freeAllOwners();
		printf("Goodbye!\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < pokedexNodes.size; i++) {
		menuItems[i].prompt = pokedexNodes.nodes[i]->data->name;
		menuItems[i].MenuAction = NULL;
		menuItems[i].nextMenuIndex = (MenuIndex)-1;
		menuItems[i].styleType = ITEM_STYLE;
	}
	if (pokedexNodes.nodes) {
		free(pokedexNodes.nodes);
		pokedexNodes.nodes = NULL;
	}

	Menu pokedexMenu = {
		.items = menuItems,
		.itemCount = pokedexNodes.size,
		.styleType = NONE
	};

	return pokedexMenu;
}


PokemonNode *searchPokemonBFS(PokemonNode *root, int id) {
    if (!root) return NULL;

    PokemonNode **queue = malloc(sizeof(PokemonNode *) * INITIAL_BUFFER_SIZE);
    if (!queue) {
        return NULL;
    }

    int front = 0, rear = 0, capacity = INITIAL_BUFFER_SIZE;
    queue[rear++] = root;

    while (front < rear) {
        PokemonNode *current = queue[front++];
        if (current->data->id == id) {
			if (queue) {
				free(queue);
				queue = NULL;
			}
            return current;
        }
        if (rear == capacity) {
            capacity *= 2;
            PokemonNode **temp = realloc(queue, sizeof(PokemonNode *) * capacity);
            if (!temp) {
                if (queue) {
					free(queue);
					queue = NULL;
				}
                return NULL;
            }
            queue = temp;
        }
        if (current->left) queue[rear++] = current->left;
        if (current->right) queue[rear++] = current->right;
    }

	if (queue) {
		free(queue);
		queue = NULL;
	}
    return NULL;
}


NodeArray *initNodeArray() {
    NodeArray *na = malloc(sizeof(NodeArray));
    if (!na) {
		return NULL;
	}
    na->size = 0;
    na->capacity = INITIAL_BUFFER_SIZE;
    na->nodes = malloc(sizeof(PokemonNode *) * na->capacity);
    if (!na->nodes) {
		if (na) {
			free(na);
			na = NULL;
		}   
        return NULL;
    }
    return na;
}


void *openPokedexMenuWrapper(MenuIndex menuIndex, int choice, void *param) {
	(void)menuIndex;
	(void)choice;
	(void)param;
	return openPokedexMenu();
}

void *openPokedexMenu(void) {
    printf("Your name: ");
    char *ownerName = getDynamicInput();
    if (!ownerName) {
        printf("Invalid input. Not creating a new Pokedex.\n");
        return NULL;
    }

    trimWhitespace(ownerName);

    if (findOwnerByName(ownerName)) {
        printf("Owner '%s' already exists. Not creating a new Pokedex.\n", ownerName);
        if (ownerName) {
			free(ownerName);
			ownerName = NULL;
		}
        return NULL;
    }

    printf(
        "Choose Starter:\n"
        "1. Bulbasaur\n"
        "2. Charmander\n"
        "3. Squirtle\n"
		"Your choice: "
    );
    int choice = readIntSafe();

	int pokemonID = NONE;

	// value 1 maps to Bulbasaur ID
	if (choice == 1) pokemonID = 1;
	// value 4 maps to Charmander ID
	if (choice == 2) pokemonID = 4;
	// value 7 maps to Squirtle ID
	if (choice == 3) pokemonID = 7;

	if (pokemonID != 1 && pokemonID != 4 && pokemonID != 7) {
		printf("Starter Pokemon with ID %d is not valid.\n", choice);
		if (ownerName) {
			free(ownerName);
			ownerName = NULL;
		}
		return NULL;
	}
	managePokemonAndOwner(NULL, ownerName, pokemonID);
	if (ownerName) {
		free(ownerName);
		ownerName = NULL;
	}
	return NULL;
}


PokemonNode *createPokemonNode(const PokemonData *data) {
	if (!data) {
		return NULL;
	}

	PokemonNode *node = malloc(sizeof(PokemonNode));
	if (!node) {
		return NULL;
	}

	node->data = malloc(sizeof(PokemonData));
	if (!node->data) {
		if (node) {
			free(node);
			return NULL;
		}
	}

	*node->data = *data;

	if (data->name) {
		node->data->name = malloc(strlen(data->name) + 1);
		if (!node->data->name) {
			if (node->data) {
				free(node->data);
				node->data = NULL;
			}
			if (node) {
				free(node);
				node = NULL;
			}
			return NULL;
		}
		strcpy(node->data->name, data->name);
	} else {
		node->data->name = NULL;
	}
	node->left = node->right = NULL;
	return node;
}

void *addPokemonWrapper(MenuIndex menuIndex, int choice, void *param) {
    (void)menuIndex;
    (void)choice;
	(void)param;
    return addPokemon(currentOwner);
}

void *addPokemon(OwnerNode *owner){
	printf("Enter ID to add:\n");
	int chosenID = readIntSafe();
	char *ownerName = (char *)(owner->ownerName);
	return managePokemonAndOwner(owner, ownerName, chosenID);
}

PokemonNode *insertPokemonNode(PokemonNode *root, PokemonNode *node) {
    if (!root) return node;
    if (node->data->id < root->data->id)
        root->left = insertPokemonNode(root->left, node);
    else if (node->data->id > root->data->id)
        root->right = insertPokemonNode(root->right, node);
    return root;
}

PokemonNode *addPokemonToPokedex(OwnerNode *owner, int pokemonID) {
    if (!owner || pokemonID < 1 || pokemonID > (int)(sizeof(pokedex) / sizeof(pokedex[0]))) {
        return NULL;
    }
	if (!findPokemonByID(pokemonID)) {
        return NULL;
    }
    PokemonNode *newNode = createPokemonNode(&pokedex[pokemonID - 1]);
    if (!newNode) {
        return NULL;
    }
    if (!owner->pokedexRoot) {
        owner->pokedexRoot = newNode;
    } else {
        insertPokemonNode(owner->pokedexRoot, newNode);
    }
    return newNode;
}

void *evolvePokemonWrapper(MenuIndex menuIndex, int choice, void *param) {
    (void)menuIndex;
    (void)choice;
	(void)param;
    if (currentOwner) {
        evolvePokemon(currentOwner);
    } else {
        printf("No current Pokedex owner. Cannot evolve Pokemon.\n");
    }
	return NULL;
}

void *evolvePokemon(OwnerNode *owner) {
    if (!owner || !owner->pokedexRoot) {
        printf("Pokedex is empty.\n");
        return NULL;
    }

    int id;
    printf("Enter ID of Pokemon to evolve: ");
    id = readIntSafe();

	if (!searchPokemonBFS(owner->pokedexRoot, id)) {
        printf("Pokemon with ID %d not found.\n", id);
        return NULL;
    }

    if (pokedex[id - 1].CAN_EVOLVE == CANNOT_EVOLVE) {
        printf("%s cannot evolve.\n", pokedex[id - 1].name);
        return NULL;
    }

    owner->pokedexRoot = removeNodeBST(owner->pokedexRoot, id);

    PokemonNode *newPokemon = createPokemonNode(&pokedex[id]);
    owner->pokedexRoot = insertPokemonNode(owner->pokedexRoot, newPokemon);

    printf("Pokemon evolved from %s (ID %d) to %s (ID %d).\n",
           pokedex[id - 1].name, pokedex[id - 1].id,
           pokedex[id].name, pokedex[id].id);
	return NULL;
}

void *mergePokedexMenuWrapper(MenuIndex menuIndex, int choice, void *param) {
	(void)menuIndex;
	(void)choice;
	(void)param; 
	return mergePokedexMenu();
}


void *mergePokedexMenu(void) {
    if (!ownerHead || ownerHead->next == ownerHead) {
        printf("Not enough owners to merge.\n");
        return NULL;
    }

    printf("=== Merge Pokedexes ===\n");

    printf("Enter name of first owner: ");
    char *nameReceiving = getDynamicInput();
    trimWhitespace(nameReceiving);

    printf("Enter name of second owner: ");
    char *nameGiving = getDynamicInput();
	if (nameGiving) {
        trimWhitespace(nameGiving);
    }

    if (!nameReceiving || !nameGiving || strcmp(nameReceiving, nameGiving) == 0) {
        printf("Not enough owners to merge.\n");
        if (nameReceiving) {
			free(nameReceiving);
			nameReceiving = NULL;
		}
		if (nameGiving) {
			free(nameGiving);
			nameGiving = NULL;
		}
        return NULL;
    }

    OwnerNode *ownerReceiving = findOwnerByName(nameReceiving);
    OwnerNode *ownerGiving = findOwnerByName(nameGiving);

    if (nameReceiving) {
		free(nameReceiving);
		nameReceiving = NULL;
	}
    if (nameGiving) {
		free(nameGiving);
		nameGiving = NULL;
	}

    if (!ownerReceiving || !ownerGiving) {
        printf("Invalid owner names. Cannot proceed with merge.\n");
        return NULL;
    }

    printf("Merging %s and %s...\n", ownerReceiving->ownerName, ownerGiving->ownerName);
    return mergePokedexes(ownerReceiving, ownerGiving);
}


void *mergePokedexes(OwnerNode *ownerReceiving, OwnerNode *ownerGiving) {
	if (!ownerReceiving || !ownerGiving) {
		return NULL;
	}

	PokemonNode *copiedTree = deepCopyTree(ownerGiving->pokedexRoot);
	if (copiedTree) {
		transferPokemonBFS(copiedTree, &ownerReceiving->pokedexRoot);
		freePokemonTree(&copiedTree);
	}

	printf("Merge completed.\nOwner '%s' has been removed after merging.\n", ownerGiving->ownerName);
	removeOwnerFromCircularList(ownerGiving);
	return NULL;
}

PokemonNode *deepCopyTree(PokemonNode *root) {
	if (!root) {
		return NULL;
	}

	PokemonNode *newNode = createPokemonNode(root->data);
	if (!newNode) {
		return NULL;
	}

	newNode->left = deepCopyTree(root->left);
	newNode->right = deepCopyTree(root->right);

	return newNode;
}

void transferPokemonBFS(PokemonNode *src, PokemonNode **dest) {
	if (!src) {
		return;
	}

	PokemonNode **queue = malloc(sizeof(PokemonNode *) * INITIAL_BUFFER_SIZE);
	
	int
		front = 0,
		rear = 0,
		size = INITIAL_BUFFER_SIZE;

	queue[rear++] = src;
	while (front < rear) {
		PokemonNode *current = queue[front++];
		PokemonNode *newNode = createPokemonNode(current->data);
		if (!newNode) {
			continue;
		}

		if (*dest == NULL) {
			*dest = newNode;
		} else {
			insertPokemonNode(*dest, newNode);
		}

		if (current->left) {
			if (rear == size) {
				size *= EXPANSION_FACTOR;
				PokemonNode **tempQueue = realloc(queue, sizeof(PokemonNode *) * size);
				if (tempQueue == NULL) {
					if (queue) {
						free(queue);
						queue = NULL;
					}
					return;
				}
				queue = tempQueue;
			}
			queue[rear++] = current->left;
		}

		if (current->right) {
			if (rear == size) {
				size *= EXPANSION_FACTOR;
				PokemonNode **tempQueue = realloc(queue, sizeof(PokemonNode *) * size);
				if (tempQueue == NULL) {
					if (queue) {
						free(queue);
						queue = NULL;
					}
					return;
				}
				queue = tempQueue;
			}
			queue[rear++] = current->right;
		}
	}
	if (queue) {
		free(queue);
		queue = NULL;
	}
	return;
}

void printPokemonNode(PokemonNode *node) {
	if (!node || !node->data) {
		return;
	}
	printf("ID: %d, ", node->data->id);
	printf("Name: %s, ", node->data->name);
	printf("Type: %s, ", getTypeName(node->data->TYPE));
	printf("HP: %d, ", node->data->hp);
	printf("Attack: %d, ", node->data->attack);
	printf("Can Evolve: %s\n", node->data->CAN_EVOLVE == CAN_EVOLVE ? "Yes" : "No");
	return;
}


OwnerNode *createOwner(const char *ownerName, PokemonNode *starter) {
    if (!ownerName) return NULL;

    OwnerNode *owner = malloc(sizeof(OwnerNode));
    if (!owner) return NULL;

    owner->ownerName = myStrdup(ownerName);
    if (!owner->ownerName) {
        free(owner);
        return NULL;
    }

    owner->pokedexRoot = starter;
    owner->next = owner->prev = NULL;

    return owner;
}


void *enterExistingPokedexMenu(MenuIndex menuIndex, int choice, void *param) {
	(void)menuIndex;
	(void)choice;
	(void)param;
    if (!ownerHead) {
        printf("No existing Pokedexes.\n");
        return menuNavigator(MAIN_MENU, NULL);
    }

    int ownerCount = 0;
    OwnerNode *current = ownerHead;
    do {
        ownerCount++;
        current = current->next;
    } while (current != ownerHead);

    MenuItem *ownerMenuItems = malloc(ownerCount * sizeof(MenuItem));
    if (!ownerMenuItems) {
        return menuNavigator(MAIN_MENU, NULL);
    }

    current = ownerHead;
    for (int i = 0; i < ownerCount; i++) {
        ownerMenuItems[i].prompt = current->ownerName;
        ownerMenuItems[i].MenuAction = NULL;
        ownerMenuItems[i].nextMenuIndex = (MenuIndex)-1;
        ownerMenuItems[i].styleType = NONE;
        current = current->next;
    }

    Menu ownerMenu = { "Existing Pokedexes", ownerMenuItems, ownerCount, UNBORDERED_STYLE };
    printMenu(&ownerMenu);

    printf("Choose a Pokedex by number: ");
    int chosenOwner = readIntSafe();

    if (chosenOwner < 1 || chosenOwner > ownerCount) {
        printf("Invalid input.\n");
       	if (ownerMenuItems) {
			free(ownerMenuItems);
			ownerMenuItems = NULL;
		}
        return menuNavigator(MAIN_MENU, NULL);
    }

    current = ownerHead;
    for (int i = 1; i < chosenOwner; ++i) {
        current = current->next;
    }

    currentOwner = current;

	if (ownerMenuItems) {
		free(ownerMenuItems);
		ownerMenuItems = NULL;
	}

    printf("\nEntering %s's Pokedex...\n", current->ownerName);
    return menuNavigator(EXISTING_POKEDEX_MENU, NULL);
}


void *deletePokedexWrapper(MenuIndex menuIndex, int choice, void *param) {
	(void)menuIndex;
	(void)choice;
	(void)param;
	if (!ownerHead) {
		printf("No existing Pokedexes to delete.\n");
		return NULL;
	}
	return deletePokedex();
}


void *deletePokedex() {
    if (!ownerHead) {
        printf("No Pokedexes to delete.\n");
        return NULL;
    }

    int ownerCount = 0;
    OwnerNode *current = ownerHead;
    do {
        ownerCount++;
        current = current->next;
    } while (current != ownerHead);

    printf("Choose a Pokedex to delete by number: ");
    int choice = readIntSafe();

    if (choice < 1 || choice > ownerCount) {
        printf("Invalid choice. No Pokedex deleted.\n");
        return NULL;
    }

    current = ownerHead;
    for (int i = 1; i < choice; i++) {
        current = current->next;
    }

    if (current == ownerHead && current->next == ownerHead) {
        ownerHead = NULL;
    } else {
        current->prev->next = current->next;
        current->next->prev = current->prev;
        if (current == ownerHead) {
            ownerHead = current->next;
        }
    }

    if (current->ownerName) {
        free(current->ownerName);
        current->ownerName = NULL;
    }
    if (current) {
        free(current);
        current = NULL;
    }
    printf("Pokedex deleted.\n");
    return NULL;
}


void swapOwnerData(OwnerNode *a, OwnerNode *b) {
	if (!a || !b) return;

	char *tempName = a->ownerName;
	PokemonNode *tempPokedex = a->pokedexRoot;

	a->ownerName = b->ownerName;
	a->pokedexRoot = b->pokedexRoot;

	b->ownerName = tempName;
	b->pokedexRoot = tempPokedex;
	return;
}

int compareOwners(const OwnerNode *A, const OwnerNode *B) {
	return compareStrings(A->ownerName, B->ownerName);
}

void *sortOwnersWrapper(MenuIndex menuIndex, int choice, void *param) {
	(void)menuIndex;
	(void)choice;
	(void)param;
	return sortOwners();
}

void *sortOwners(void) {
    if (!ownerHead || ownerHead->next == ownerHead) {
        printf("0 or 1 owners only => no need to sort.\n");
        return NULL;
    }

    int swapped;
    OwnerNode *current;
    OwnerNode *end = NULL;

    do {
        swapped = 0;
        current = ownerHead;

        while (current->next != end && current->next != ownerHead) {
            if (compareStrings(current->ownerName, current->next->ownerName) > 0) {
                swapOwnerData(current, current->next);
                swapped = 1;
            }
            current = current->next;
        }
        end = current;
    } while (swapped);
    OwnerNode *tail = ownerHead;
    while (tail->next != ownerHead) {
        tail = tail->next;
    }
    tail->next = ownerHead;
    ownerHead->prev = tail;
    printf("Owners sorted by name.\n");
    return NULL;
}


void *managePokemonAndOwner(OwnerNode *owner, char *ownerName, int pokemonID) {
    if (pokemonID < 1 || pokemonID > (int)(sizeof(pokedex) / sizeof(pokedex[0]))) {
        printf("Invalid Pokemon ID.\n");
        if (ownerName) {
            free(ownerName);
            ownerName = NULL;
        }
        return NULL;
    }

    if (!owner) {
        if (!ownerName) {
            printf("Owner name is required to create a new Pokedex.\n");
            return NULL;
        }

        OwnerNode *newOwner = createOwner(ownerName, NULL);
        if (!newOwner) {
            printf("Failed to create owner.\n");
            if (ownerName) {
				free(ownerName);
				ownerName = NULL;
			}
            return NULL;
        }

        linkOwnerInCircularList(newOwner);

        PokemonNode *starterNode = addPokemonToPokedex(newOwner, pokemonID);
        if (!starterNode) {
            printf("Failed to add starter Pokemon to the new Pokedex.\n");
            return NULL;
        }

        const PokemonData *globalPokemon = &pokedex[pokemonID - 1];
        printf(
			"Pokedex for %s created with starter Pokemon %s (ID: %d).\n",
            ownerName, globalPokemon->name, pokemonID
		);
		return NULL;
		
    }

	if (!findPokemonByID(pokemonID)) {
        return NULL;
    }

    PokemonNode *newNode = addPokemonToPokedex(owner, pokemonID);
    if (newNode) {
        printf("Pokemon %s (ID %d) added.\n", newNode->data->name, pokemonID);
    } else {
        printf("Failed to add Pokemon to the Pokedex.\n");
    }
	return NULL;
}


void *freePokemonWrapper(MenuIndex menuIndex, int choice, void *param) {
    (void)menuIndex;
    (void)choice;
    (void)param;
    return freePokemon(currentOwner);
}

void *freePokemon(OwnerNode *owner) {
	if (!owner || !owner->pokedexRoot) {
		printf("No Pokemon to release.\n");
		return NULL;
	}

	int id;
	printf("Enter Pokemon ID to release: ");
	id = readIntSafe();

	owner->pokedexRoot = removePokemonByID(owner->pokedexRoot, id);
	return NULL;
}

void freePokemonNode(PokemonNode **node) {
    if (!node || !*node) {
        return;
    }

    if ((*node)->data) {
        if ((*node)->data->name) {
            free((*node)->data->name);
            (*node)->data->name = NULL;
        }
        free((*node)->data);
        (*node)->data = NULL;
    }

    free(*node);
    *node = NULL;
}


void freePokemonTree(PokemonNode **root) {
    if (!root || !*root) {
        return;
    }

    freePokemonTree(&(*root)->left);
    freePokemonTree(&(*root)->right);
    freePokemonNode(root);
}

void freeOwnerNode(OwnerNode *owner) {
	if (!owner) return;

	if (owner->ownerName) {
		free(owner->ownerName);
		owner->ownerName = NULL;
	}
	freePokemonTree(&owner->pokedexRoot);
	if (owner) {
		free(owner);
		owner = NULL; 
	}
	return;
}

void freeAllOwners(void) {
    if (!ownerHead) {
        return;
	}
	OwnerNode *currentOwner = ownerHead;
	do {
		if (!currentOwner->ownerName) {
			if (!keepGoing) {
				return;
			} else {
				break;
			}
		}
		OwnerNode *next = currentOwner->next;
		freeOwnerNode(currentOwner);
		currentOwner = next;
	} while (currentOwner != ownerHead);
	ownerHead = NULL;
	return;
}


PokemonQueue *createPokemonQueue() {
   PokemonQueue *pokemQueue = (PokemonQueue*)malloc(sizeof(PokemonQueue));
   pokemQueue->front = NULL;
   pokemQueue->rear = NULL;
   return pokemQueue;
}

NodeQueue* createNode(PokemonNode* pokeNode) {
   NodeQueue *node = (NodeQueue*)malloc(sizeof(NodeQueue));
   node->next = NULL;
   node->pokeNode = pokeNode;
   return node;
}

void queueExtend(PokemonQueue *pokemQueue, PokemonNode* pokeNode) {
   NodeQueue* node = createNode(pokeNode);
   if (pokemQueue->rear == NULL) {
	  pokemQueue->front = node;
	  pokemQueue->rear = node;
	  return;
   }
   pokemQueue->rear->next = node;
   pokemQueue->rear = node;
   return;
}

int isEmptyQueue(PokemonQueue* queue) {
   return (queue->front == NULL);
}

PokemonNode* removeFromQueue(PokemonQueue *queue) {
	if (isEmptyQueue(queue)){
		return NULL;
	}
	NodeQueue *node = queue->front;
	queue->front = queue->front->next;
	if (queue->front == NULL) {
		queue->rear = NULL;
	}
	PokemonNode* pokeNode = node->pokeNode;
	if (node) {
		free(node);
		node = NULL;
	}
	return pokeNode;
}

void linkOwnerInCircularList(OwnerNode *newOwner) {
	if (!newOwner) return;

	if (!ownerHead) {
		ownerHead = newOwner;
		ownerHead->next = ownerHead;
		ownerHead->prev = ownerHead;
	} else {
		OwnerNode *tail = ownerHead->prev;
		tail->next = newOwner;
		newOwner->prev = tail;
		newOwner->next = ownerHead;
		ownerHead->prev = newOwner;
	}
	return;
}

void removeOwnerFromCircularList(OwnerNode *toRemove) {
    if (!toRemove || !ownerHead) {
		return;
	}

    OwnerNode *prev = toRemove->prev;
    OwnerNode *next = toRemove->next;

    prev->next = next;
    next->prev = prev;

    if (toRemove->ownerName) {
        free(toRemove->ownerName);
        toRemove->ownerName = NULL;
    }
    freePokemonTree(&toRemove->pokedexRoot);
	if (toRemove) {
		free(toRemove);
		toRemove = NULL;
	}
}


void *printOwnersCircularWrapper(MenuIndex menuIndex, int choice, void *param) {
	(void)menuIndex;
	(void)choice;
	(void)param;
	return printOwnersCircular();
}

void *printOwnersCircular(void) {
    if (!ownerHead) {
        printf("No owners.\n");
        return NULL;
    }

	printf("Enter direction (F or B): ");
    char *directionInput = (char *)getDynamicInput();
    if (!directionInput) {
        printf("Invalid input.\n");
        return NULL;
    }
	// handle uppercase and lowercase
    char direction = toupper(*directionInput);
    free(directionInput);
    directionInput = NULL;

    if (direction != 'F' && direction != 'B') {
		printf("Invalid input.\n");
        // printf("Invalid direction. Please choose 'F' or 'B'. Returning to menu.\n");
        return NULL;
    }

    printf("How many prints? ");
    int repeatCount = readIntSafe();
    if (repeatCount <= 0) {
        return NULL;
    }

    OwnerNode *current = ownerHead;
    for (int i = 0; i < repeatCount; i++) {
        printf("[%d] %s\n", i + 1, current->ownerName);
        current = (direction == 'F') ? current->next : current->prev;
    }
    printf("\n");
    return NULL;
}


void *displayAlphabeticalWrapper(MenuIndex menuIndex, int choice, void *param) {
    (void)menuIndex;
    (void)choice;
    (void)param;
    if (currentOwner) {
        displayAlphabetical(currentOwner->pokedexRoot);
    } else {
        printf("No current Pokedex owner. Cannot display in alphabetical order.\n");
    }
	return NULL;
}

void displayAlphabetical(PokemonNode *root) {
    if (!root) {
        printf("Pokedex is empty.\n");
        return;
    }

    NodeArray nodeArray;
    nodeArray.size = 0;
    nodeArray.capacity = INITIAL_BUFFER_SIZE;
    nodeArray.nodes = malloc(sizeof(PokemonNode *) * nodeArray.capacity);
    if (!nodeArray.nodes) {
        return;
    }

    collectAll(root, &nodeArray);

    for (int i = 0; i < nodeArray.size - 1; i++) {
        for (int j = i + 1; j < nodeArray.size; j++) {
            if (strcmp(nodeArray.nodes[i]->data->name, nodeArray.nodes[j]->data->name) > 0) {
                PokemonNode *temp = nodeArray.nodes[i];
                nodeArray.nodes[i] = nodeArray.nodes[j];
                nodeArray.nodes[j] = temp;
            }
        }
    }

    for (int i = 0; i < nodeArray.size; i++) {
        printPokemonNode(nodeArray.nodes[i]);
    }

	if (nodeArray.nodes) {
		free(nodeArray.nodes);
		nodeArray.nodes = NULL;
	}
	return;
}

void *displayBFSWrapper(MenuIndex menuIndex, int choice, void *param) {
    (void)menuIndex;
    (void)choice;
    (void)param;
    if (currentOwner) {
        displayBFS(currentOwner->pokedexRoot);
		return NULL;
    } else {
        printf("No current Pokedex owner. Cannot display BFS.\n");
    }
	return NULL;
}

void displayBFS(PokemonNode *root) {
    if (!root) {
        printf("Pokedex is empty.\n");
        return;
    }

    PokemonQueue *queue = createPokemonQueue();
    if (!queue) {
        // fprintf(stderr, "Memory allocation failed for BFS queue.\n");
        return;
    }

    queueExtend(queue, root);

    while (!isEmptyQueue(queue)) {
        PokemonNode *current = removeFromQueue(queue);
        printPokemonNode(current);
        if (current->left) queueExtend(queue, current->left);
        if (current->right) queueExtend(queue, current->right);
    }

	if (queue) {
		free(queue);
		queue = NULL;
	}
	return;
}

void BFSGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (!root || !visit) return;

    PokemonQueue *queue = createPokemonQueue();
    if (!queue) {
        // printf("Memory allocation failed for BFS queue.\n");
        return;
    }

    queueExtend(queue, root);

    while (!isEmptyQueue(queue)) {
        PokemonNode *current = removeFromQueue(queue);
        visit(current);
        if (current->left) queueExtend(queue, current->left);
        if (current->right) queueExtend(queue, current->right);
    }

	if (queue) {
		free(queue);
		queue= NULL;
	}
    return;
}

void *preOrderTraversalWrapper(MenuIndex menuIndex, int choice, void *param) {
    (void)menuIndex;
    (void)choice;
    (void)param;
    if (currentOwner) {
        preOrderTraversal(currentOwner->pokedexRoot);
    } else {
        printf("No current Pokedex owner. Cannot perform pre-order traversal.\n");
    }
	return NULL;
}

void preOrderTraversal(PokemonNode *root){
	if (root==NULL){
	  printf("Pokedex is empty.\n");
	  return;
   }
	preOrderGeneric(root);
	return;
}

void preOrderGeneric(PokemonNode *root){
    if (!root) return;
    printPokemonNode(root);
    preOrderGeneric(root->left);
    preOrderGeneric(root->right);
	return;
}

void *inOrderTraversalWrapper(MenuIndex menuIndex, int choice, void *param) {
    (void)menuIndex;
    (void)choice;
    (void)param;
    if (currentOwner) {
        inOrderTraversal(currentOwner->pokedexRoot);
    } else {
        printf("No current Pokedex owner. Cannot perform in-order traversal.\n");
    }
	return NULL;
}


void inOrderTraversal(PokemonNode *root){
	if (root==NULL){
	  printf("Pokedex is empty.\n");
	  return;
   }
	inOrderGeneric(root);
	return;
}

void inOrderGeneric(PokemonNode *root){
    if (!root) return;
    inOrderGeneric(root->left);
    printPokemonNode(root);
    inOrderGeneric(root->right);
	return;
}


void *postOrderTraversalWrapper(MenuIndex menuIndex, int choice, void *param) {
    (void)menuIndex;
    (void)choice;
    (void)param;
    if (currentOwner) {
        postOrderTraversal(currentOwner->pokedexRoot);
    } else {
        printf("No current Pokedex owner. Cannot perform post-order traversal.\n");
    }
	return NULL;
}

void postOrderTraversal(PokemonNode *root){
	if (root==NULL){
	  printf("Pokedex is empty.\n");
	  return;
   }
	postOrderGeneric(root);
	return;
}

void postOrderGeneric(PokemonNode *root){
    if (!root) return;
    postOrderGeneric(root->left);
    postOrderGeneric(root->right);
    printPokemonNode(root);
	return;
}

PokemonNode *removePokemonByID(PokemonNode *root, int id) {
    if (!root) {
        printf("Pokedex is empty.\n");
        return NULL;
    }

    PokemonNode *trashNode = searchPokemonBFS(root, id);
    if (!trashNode) {
        printf("Pokemon with ID %d not found in the Pokedex.\n", id);
        return root;
    }

    PokemonNode *parent = findParent(root, trashNode);

    if (!trashNode->left && !trashNode->right) {
        if (!parent) {
            printf("Removing Pokemon %s (ID %d).\n", trashNode->data->name, id);
            freePokemonNode(&trashNode);
            return NULL;
        }
        if (parent->left == trashNode) {
            parent->left = NULL;
        } else {
            parent->right = NULL;
        }
        printf("Removing Pokemon %s (ID %d).\n", trashNode->data->name, id);
        freePokemonNode(&trashNode);
        return root;
    }

    if (trashNode->left && !trashNode->right) {
        if (!parent) {
            printf("Removing Pokemon %s (ID %d).\n", trashNode->data->name, id);
            PokemonNode *newRoot = trashNode->left;
            freePokemonNode(&trashNode);
            return newRoot;
        }
        if (parent->left == trashNode) {
            parent->left = trashNode->left;
        } else {
            parent->right = trashNode->left;
        }
        printf("Removing Pokemon %s (ID %d).\n", trashNode->data->name, id);
        freePokemonNode(&trashNode);
        return root;
    }

    if (!trashNode->left && trashNode->right) {
        if (!parent) {
            printf("Removing Pokemon %s (ID %d).\n", trashNode->data->name, id);
            PokemonNode *newRoot = trashNode->right;
            freePokemonNode(&trashNode);
            return newRoot;
        }
        if (parent->left == trashNode) {
            parent->left = trashNode->right;
        } else {
            parent->right = trashNode->right;
        }
        printf("Removing Pokemon %s (ID %d).\n", trashNode->data->name, id);
        freePokemonNode(&trashNode);
        return root;
    }

    if (trashNode->left && trashNode->right) {
        PokemonNode *successor = findMin(trashNode->right);
        trashNode->data = successor->data;
        trashNode->right = removePokemonByID(trashNode->right, successor->data->id); // Remove successor
        return root;
    }
    return root;
}

PokemonNode *removeNodeBST(PokemonNode *root, int id) {
	if (!root) return NULL;

	if (id < root->data->id) {
		root->left = removeNodeBST(root->left, id);
	} else if (id > root->data->id) {
		root->right = removeNodeBST(root->right, id);
	} else {
		if (!root->left) {
			PokemonNode *currentPokemonSelection = root->right;
			freePokemonNode(&root);
			return currentPokemonSelection;
		} else if (!root->right) {
			PokemonNode *currentPokemonSelection = root->left;
			freePokemonNode(&root);
			return currentPokemonSelection;
		}

		PokemonNode *successor = root->right;
		PokemonNode *parent = root;

		while (successor->left) {
			parent = successor;
			successor = successor->left;
		}

		if (parent->left == successor) {
			parent->left = successor->right;
		} else {
			parent->right = successor->right;
		}

		successor->left = root->left;
		successor->right = root->right;

		freePokemonNode(&root);
		return successor;
	}

	return root;
}

int compareByNameNode(const void *a, const void *b) {
    const PokemonNode *nodeA = *(const PokemonNode **)a;
    const PokemonNode *nodeB = *(const PokemonNode **)b;

    if (!nodeA || !nodeA->data) return -1;
    if (!nodeB || !nodeB->data) return 1;

    return strcmp(nodeA->data->name, nodeB->data->name);
}

void *pokemonFightWrapper(MenuIndex menuIndex, int choice, void *param) {
    (void)menuIndex;
    (void)choice;
    (void)param;
    if (currentOwner) {
        return pokemonFight(currentOwner);
    } else {
        printf("No current Pokedex owner. Cannot initiate Pokemon fight.\n");
    }
	return NULL;
}

void *pokemonFight(OwnerNode *owner) {
	if (!owner->pokedexRoot) {
		printf("Pokedex is empty.\n");
		return NULL;
	}

	printf("Enter ID of the first Pokemon: ");
	int id1 = readIntSafe();

	printf("Enter ID of the second Pokemon: ");
	int id2 = readIntSafe();

	PokemonNode *pokemon1 = searchPokemonBFS(owner->pokedexRoot, id1);
	PokemonNode *pokemon2 = searchPokemonBFS(owner->pokedexRoot, id2);

	if (!pokemon1 || !pokemon2) {
		printf("One or both Pokemon IDs not found.\n");
		return NULL;
	}

	double score1 = pokemon1->data->attack * 1.5 + pokemon1->data->hp * 1.2;
	double score2 = pokemon2->data->attack * 1.5 + pokemon2->data->hp * 1.2;

	printf("Pokemon 1: %s (Score = %.2f)\n", pokemon1->data->name, score1);
	printf("Pokemon 2: %s (Score = %.2f)\n", pokemon2->data->name, score2);

	if (score1 > score2) {
		printf("%s wins!\n", pokemon1->data->name);
	} else if (score2 > score1) {
		printf("%s wins!\n", pokemon2->data->name);
	} else {
		printf("It's a tie!\n");
	}
	return NULL;
}

int compareStrings(const char *str1, const char *str2) {
    if (!str1 || !str2) return 0;

    while (*str1 && *str2) {
        if (tolower((unsigned char)*str1) != tolower((unsigned char)*str2)) return 0;
        str1++;
        str2++;
    }

    return *str1 == '\0' && *str2 == '\0';
}

OwnerNode *findOwnerByName(const char *name) {
    if (!name) {
		return NULL;
	}

    OwnerNode *current = ownerHead;
    if (!current) {
		return NULL;
	}

    do {
        if (!current->ownerName) {
            current = current->next;
            continue;
        }
        if (compareStrings(current->ownerName, name)) {
            return current;
        }
        current = current->next;
    } while (current && current != ownerHead);

    return NULL;
}

PokemonNode *findMin(PokemonNode *root){
	while (root && root->left){
		root = root->left;
	}
	return root;
}

PokemonNode *findParent(PokemonNode *root, PokemonNode *kid){
	if (root == NULL || kid==NULL || root == kid) return NULL;
	if (root->left == kid || root->right == kid) return root;
	if (kid->data->id > root->data->id){
		return findParent(root->right,kid);
	}
	else return findParent(root->left,kid);
}

void collectAll(PokemonNode *root, NodeArray *nodeArray) {
    if (!root || !nodeArray) return;

    if (nodeArray->size >= nodeArray->capacity) {
        nodeArray->capacity *= 2;
        PokemonNode **temp = realloc(nodeArray->nodes, sizeof(PokemonNode *) * nodeArray->capacity);
        if (!temp) {
			if (nodeArray->nodes) {
				free(nodeArray->nodes);
            	nodeArray->nodes = NULL;
			}
            return;
        }
        nodeArray->nodes = temp;
    }

    nodeArray->nodes[nodeArray->size++] = root;

    collectAll(root->left, nodeArray);
    collectAll(root->right, nodeArray);
	return;
}

void cleanupResources(void **param, Menu *ownerMenu, int *keepGoing) {
	if (*param) {
		free(*param);
		*param = NULL;
	}
	if (ownerMenu->items) {
		free((void *)ownerMenu->items);
		ownerMenu->items = NULL;
	}
	if (keepGoing) {
		stop();
		freeAllOwners();
	}
}

int main(void) {
	do {
		void *returnedSelf = menuNavigator(MAIN_MENU, NULL);
		if (keepGoing && returnedSelf == funcPtr) {
			continue;
		}
		stop();
	} while(keepGoing);
	freeAllOwners();
	printf("Goodbye!\n");
    return EXIT_SUCCESS;
}

