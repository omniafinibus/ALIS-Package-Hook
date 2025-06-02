#ifndef MAIN
#define MAIN

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Commands
#define READ_COMMAND "pacman -Qi | grep -iE 'Name            :|Install Reason  :|Packager        :'"
#define TIME_COMMAND "date"

// Object sizes
#define BUFFER_LENGTH 20
#define LABEL_LENGTH 6
#define LIST_SIZE 50000

// Labels
#define YAY 'Y'
#define PACMAN 'P'
#define LABEL_NAME "\nName            : "
#define LABEL_PACK "\nPackager        : "
#define LABEL_REASON "\nInstall Reason  : "
#define FLAG_CHAR '~'
#define LABEL_DATE "~DATE"
#define LABEL_PACMAN "~PACM"
#define LABEL_AUR "~AURP"

// State machine
#define STATE_BREAK 0
#define STATE_NAME 1
#define STATE_PACK 2
#define STATE_REASON 3

typedef struct {
    char source;
    bool explicit;
    char name[100];
}package;

// Function declarations
bool add_entry(char *, package *, size_t *);
bool is_it_aur(char *);
bool buffer_contains_eol(char *);
void update_buffer(char *, char, size_t);
uint8_t get_state(char *, uint8_t);

// Config contents
const char CONTENTS[] = "# Arch Linux Install Script (alis) configuration file\n#\n# Auto generated on ~DATE\nLOG_TRACE=\"true\"\nLOG_FILE=\"true\"\n\n# pacman\nPACKAGES_PACMAN_INSTALL=\"true\"\nPACKAGES_PACMAN_INSTALL_PIPEWIRE=\"false\"\nPACKAGES_PACMAN=\"~PACM\"\nPACKAGES_PACMAN_CUSTOM_REPOSITORIES=\"\n#[custom]\n#SigLevel = Optional TrustAll\n#Server = file:///home/custompkgs\n\"\n\n## AUR utility and AUR packages to install\nPACKAGES_AUR_INSTALL=\"true\"\nPACKAGES_AUR_COMMAND=\"yay\"\nPACKAGES_AUR=\"~AURP\"\n\n# systemd (\"+\", \"-\", \"!\")\n## SYSTEMD_UNITS systemd units to enable or disable.\nSYSTEMD_UNITS=\"+bluetooth.service +NetworkManager.service\"\n\n## Redundant variables\nPACKAGES_FLATPAK_INSTALL=\"false\"\nPACKAGES_FLATPAK=\"\"\nPACKAGES_SDKMAN_INSTALL=\"false\"\nPACKAGES_SDKMAN=\"\"";

#endif