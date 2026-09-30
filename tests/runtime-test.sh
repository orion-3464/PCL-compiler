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
if ! command -v timeout >/dev/null 2>&1; then
    printf 'GNU timeout is required for runtime tests.\n' >&2
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

work_dir=$(mktemp -d "${TMPDIR:-$script_dir}/.pcl-runtime.XXXXXX") || exit 2
trap 'rm -rf -- "$work_dir"' EXIT

passed=0
failed=0
skipped=0
run_timeout=${PCL_RUN_TIMEOUT:-10s}
for source in "${programs[@]}"; do
    name=${source##*/}
    name=${name%.pcl}
    expected="$programs_dir/$name.result"
    input="$programs_dir/$name.input"

    if [[ ! -e $expected ]]; then
        if [[ -L $expected ]]; then
            printf 'FAIL  %s (broken .result symlink)\n' "$name"
            ((failed += 1))
        else
            printf 'SKIP  %s (no .result)\n' "$name"
            ((skipped += 1))
        fi
        continue
    fi
    if [[ -L $input && ! -e $input ]]; then
        printf 'FAIL  %s (broken .input symlink)\n' "$name"
        ((failed += 1))
        continue
    fi

    case_dir="$work_dir/$name"
    mkdir -p -- "$case_dir"
    cp -- "$source" "$case_dir/$name.pcl"
    (cd -- "$project_root" && "$compiler" -o "$case_dir/program" "$case_dir/$name.pcl") \
        >"$case_dir/compile.stdout" 2>"$case_dir/compile.stderr"
    status=$?
    if ((status != 0)) || [[ ! -x $case_dir/program ]]; then
        printf 'FAIL  %s (compiler exit %d)\n' "$name" "$status"
        if ((status == 0)); then
            printf '  Compiler reported success but produced no executable.\n'
        fi
        sed -n '1,20p' "$case_dir/compile.stderr"
        ((failed += 1))
        continue
    fi

    if [[ -e $input ]]; then
        timeout --kill-after=2s "$run_timeout" "$case_dir/program" \
            <"$input" >"$case_dir/actual" 2>"$case_dir/runtime.stderr"
    else
        timeout --kill-after=2s "$run_timeout" "$case_dir/program" \
            </dev/null >"$case_dir/actual" 2>"$case_dir/runtime.stderr"
    fi
    status=$?

    if ((status != 0)); then
        printf 'FAIL  %s (program exit %d; timeout is 124)\n' "$name" "$status"
        sed -n '1,20p' "$case_dir/runtime.stderr"
        ((failed += 1))
    elif cmp -s -- "$expected" "$case_dir/actual"; then
        printf 'PASS  %s\n' "$name"
        ((passed += 1))
    else
        printf 'FAIL  %s (output differs from .result)\n' "$name"
        diff -u -- "$expected" "$case_dir/actual" | sed -n '1,60p'
        ((failed += 1))
    fi
done

printf '\nRuntime: %d passed, %d failed, %d skipped, %d total\n' \
    "$passed" "$failed" "$skipped" "${#programs[@]}"
((failed == 0))
