R"bash(
_archivarius() {
	local cur prev subcmd commands keys_restore keys_archive keys_list keys_listfiles keys_remove keys_test keys_version
	local i key used_keys filtered_keys filtered_keys_with_eq

	# Initialize completion variables (Bash 4+ style)
	COMPREPLY=()
	cur="${COMP_WORDS[COMP_CWORD]}"
	prev="${COMP_WORDS[COMP_CWORD - 1]}"

	# Subcommands
	commands="archive list list-files restore remove test version"

	# Possible keys per subcommand (based on usage)
	keys_restore="archive name id target-dir prefix password"
	keys_archive="name"
	keys_list="name archive password"
	keys_listfiles="name archive password id"
	keys_remove="archive name id password"  # Inferred for remove
	keys_test="archive name"
	keys_version=""  # No params

	if [[ ${COMP_CWORD} -eq 1 ]]; then
		# Completing subcommand
		COMPREPLY=( $(compgen -W "${commands}" -- "${cur}") )
		# Add space after subcommand
		compopt +o nospace
		return 0
	fi

	subcmd="${COMP_WORDS[1]}"

	# Determine if completing a value (after =)
	if [[ ${prev} == *"=" ]]; then
		# Strip the trailing = to get the key
		key="${prev%=}"

		# Value completion: files for path params, default otherwise
		case "${subcmd}" in
			restore|list|list-files|remove|test)
				case "${key}" in
					archive|target-dir|prefix)
						# File/directory completion, no space after
						COMPREPLY=( $(compgen -f -- "${cur}") )
						compopt -o nospace
						;;
					name)
						# Task names: no dynamic completion, fall back to filenames or default
						COMPREPLY=( $(compgen -o default -- "${cur}") )
						;;
					id)
						# Version IDs: numeric-ish, but default for now
						COMPREPLY=( $(compgen -o default -- "${cur}") )
						;;
					password)
						# No completion for password
						return 0
						;;
				esac
				;;
			*)
				# Default for other subcommands
				COMPREPLY=( $(compgen -o default -- "${cur}") )
				;;
		esac
		return 0
	fi

	# Otherwise, completing a new key
	# Collect used keys to avoid duplicates (parse previous words)
	used_keys=()
	for ((i = 2; i < COMP_CWORD; i++)); do  # Start after subcmd
		word="${COMP_WORDS[i]}"
		if [[ ${word} != *"=" ]]; then
			# If not ending with =, it's likely a key (incomplete value)
			used_keys+=("${word%%=*}")  # Strip =value if partial
		else
			# Full key=value
			used_keys+=("${word%%=*}")  # Extract key
		fi
	done

	# Get allowed keys for this subcommand
	case "${subcmd}" in
		restore) allowed_keys="${keys_restore}" ;;
		archive) allowed_keys="${keys_archive}" ;;
		list) allowed_keys="${keys_list}" ;;
		list-files) allowed_keys="${keys_listfiles}" ;;
		remove) allowed_keys="${keys_remove}" ;;
		test) allowed_keys="${keys_test}" ;;
		version) allowed_keys="${keys_version}" ;;
		*) allowed_keys="" ;;  # Unknown subcmd, no keys
	esac

	if [[ -z "${allowed_keys}" ]]; then
		# No params expected
		return 0
	fi

	# Filter allowed keys not already used and append = for each
	filtered_keys_with_eq=""
	for k in ${allowed_keys}; do
		if ! [[ " ${used_keys[*]} " =~ " ${k} " ]]; then
			filtered_keys_with_eq="${filtered_keys_with_eq} ${k}="
		fi
	done

	# Complete with unused keys followed by =
	COMPREPLY=( $(compgen -W "${filtered_keys_with_eq}" -- "${cur}") )
	# Prevent space after key=
	compopt -o nospace
	return 0
}

# Register the completion
complete -F _archivarius archivarius

)bash"
