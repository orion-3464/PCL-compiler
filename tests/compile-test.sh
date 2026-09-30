#!/usr/bin/env bash
set -u

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
programs_arg=${1:-${PCL_PROGRAMS_DIR:-"$script_dir/../../ntua_compilers/pcl/programs"}}
compiler_arg=${2:-${PCL_COMPILER:-"$script_dir/../pclc"}}

if [[ ! -d $programs_arg ]]; then
    printf 'Program directory not found: %s\n' "$programs_arg" >&2
    exit 2
fi
if [[ ! -x $compiler_arg ]]; then
    printf 'Compiler is not executable: %s\n' "$compiler_arg" >&2
    exit 2
fi

programs_dir=$(cd -- "$programs_arg" && pwd)
compiler=$(cd -- "$(dirname -- "$compiler_arg")" && pwd)/$(basename -- "$compiler_arg")
project_arg=${3:-${PCL_PROJECT_ROOT:-"$(dirname -- "$compiler")"}}
if [[ ! -d $project_arg ]]; then
    printf 'Project root not found: %s\n' "$project_arg" >&2
    exit 2
fi
project_root=$(cd -- "$project_arg" && pwd)

shopt -s nullglob
programs=("$programs_dir"/*.pcl)
if ((${#programs[@]} == 0)); then
    printf 'No .pcl files in %s\n' "$programs_dir" >&2
    exit 2
fi

work_dir=$(mktemp -d "${TMPDIR:-$script_dir}/.pcl-compile.XXXXXX") || exit 2
trap 'rm -rf -- "$work_dir"' EXIT

passed=0
failed=0
for source in "${programs[@]}"; do
    name=${source##*/}
    name=${name%.pcl}
    case_dir="$work_dir/$name"
    mkdir -p -- "$case_dir"
    cp -- "$source" "$case_dir/$name.pcl"

    # The compiler writes .imm/.asm next to its input. Compile the copy.
    (cd -- "$project_root" && "$compiler" -o "$case_dir/program" "$case_dir/$name.pcl") \
        >"$case_dir/compile.stdout" 2>"$case_dir/compile.stderr"
    status=$?

    if ((status == 0)) && [[ -x $case_dir/program ]]; then
        printf 'PASS  %s\n' "$name"
        ((passed += 1))
    else
        printf 'FAIL  %s (compiler exit %d)\n' "$name" "$status"
        if ((status == 0)); then
            printf '  Compiler reported success but produced no executable.\n'
        fi
        sed -n '1,20p' "$case_dir/compile.stderr"
        ((failed += 1))
    fi
done

printf '\nCompile: %d passed, %d failed, %d total\n' \
    "$passed" "$failed" "${#programs[@]}"
((failed == 0))
