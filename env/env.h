#ifndef ENV_H
#define ENV_H

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief Load environment variables from a .env file.
	 * @param path Path to .env file.
	 * @return 0 on success, -1 on failure.
	 */
	int env_load(const char* path);

	/**
	 * @brief Get value of a loaded environment variable.
	 * @param key Environment variable key.
	 * @return Value string or NULL if not found.
	 */
	const char* env_get(const char* key);

	/**
	 * @brief Inject all loaded environment variables into the process environment.
	 * @param overwrite Nonzero to overwrite existing system variables.
	 * @return 0 on success, -1 on failure.
	 */
	int env_inject_all(int overwrite);

	/**
	 * @brief Free all loaded environment variable data.
	 */
	void env_free(void);

#ifdef __cplusplus
}
#endif

#endif
