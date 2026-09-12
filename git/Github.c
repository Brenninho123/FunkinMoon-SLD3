#ifndef GIT_GITHUB_H
#define GIT_GITHUB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define GITHUB_MAX_BUFFER 1024
#define GITHUB_DEFAULT_REMOTE "origin"
#define GITHUB_DEFAULT_BRANCH "main"

typedef struct {
    char owner[128];
    char repository[128];
    char default_branch[64];
    char remote_name[64];
} github_config_t;

void github_init(github_config_t *config, const char *owner, const char *repo);
void github_set_branch(github_config_t *config, const char *branch);
void github_set_remote(github_config_t *config, const char *remote);

int github_git_add_all(void);
int github_git_commit(const char *message);
int github_git_push(const github_config_t *config, bool force);
int github_git_pull(const github_config_t *config);
int github_sync(const github_config_t *config, const char *commit_message);

#endif

#ifdef GIT_GITHUB_IMPLEMENTATION

void github_init(github_config_t *config, const char *owner, const char *repo) {
    if (!config) return;
    memset(config, 0, sizeof(github_config_t));
    
    if (owner) strncpy(config->owner, owner, sizeof(config->owner) - 1);
    if (repo) strncpy(config->repository, repo, sizeof(config->repository) - 1);
    
    strncpy(config->default_branch, GITHUB_DEFAULT_BRANCH, sizeof(config->default_branch) - 1);
    strncpy(config->remote_name, GITHUB_DEFAULT_REMOTE, sizeof(config->remote_name) - 1);
}

void github_set_branch(github_config_t *config, const char *branch) {
    if (config && branch) {
        strncpy(config->default_branch, branch, sizeof(config->default_branch) - 1);
    }
}

void github_set_remote(github_config_t *config, const char *remote) {
    if (config && remote) {
        strncpy(config->remote_name, remote, sizeof(config->remote_name) - 1);
    }
}

int github_git_add_all(void) {
    return system("git add .");
}

int github_git_commit(const char *message) {
    if (!message) return -1;
    char command[GITHUB_MAX_BUFFER];
    snprintf(command, sizeof(command), "git commit -m \"%s\"", message);
    return system(command);
}

int github_git_push(const github_config_t *config, bool force) {
    if (!config) return -1;
    char command[GITHUB_MAX_BUFFER];
    snprintf(command, sizeof(command), "git push %s %s %s",
        config->remote_name,
        config->default_branch,
        force ? "--force" : ""
    );
    return system(command);
}

int github_git_pull(const github_config_t *config) {
    if (!config) return -1;
    char command[GITHUB_MAX_BUFFER];
    snprintf(command, sizeof(command), "git pull %s %s",
        config->remote_name,
        config->default_branch
    );
    return system(command);
}

int github_sync(const github_config_t *config, const char *commit_message) {
    if (!config || !commit_message) return -1;
    
    int result = github_git_add_all();
    if (result != 0) return result;

    result = github_git_commit(commit_message);
    if (result != 0) return result;

    return github_git_push(config, false);
}

#endif
