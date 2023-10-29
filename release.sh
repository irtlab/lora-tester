#!/usr/bin/env bash
set -Eeuo pipefail

bail() {
    echo "$1";
    exit 1;
}

usage() {
    cat <<EOF
Usage: $0 [options...] <version>

Options:
  -h : Print this help message and exit
  -b : Base name (lora-tester by default)
  -c : Do not check git clone status
  -s : Do not generate PGP signatures
  -d : Discard all build artifacts (dry run)
  -g : Do not push to GitHub

Arguments:
  version : Version string (without the "v" git tag prefix)

Environment Variables:
  GITHUB_TOKEN : GitHub authentication token (required)
EOF
    exit 1;
}

while getopts "dhb:pcsgP" option; do
    case $option in
        h) usage ;;
        b) basename="$OPTARG" ;;
        d) dry_run=1 ;;
        s) sign=0 ;;
        c) check_git_status=0 ;;
        g) push_to_github=0 ;;
        *) bail "Error: Invalid option -$OPTARG" ;;
    esac
done
shift $((OPTIND-1))

[ $# -ne 1 ] && bail "Error: Missing version (see -h)"
version="$1"

orig_dir="$(pwd)"
basename="${basename:-lora-tester}"
dry_run="${dry_run:-0}"
sign="${sign:-1}"
check_git_status="${check_git_status:-1}"
push_to_github="${push_to_github:-1}"

if [ $dry_run -eq 0 ]; then
    if [ $push_to_github -eq 1 ] ; then
        # Make sure we have a GitHub token.
        [ -z "${GITHUB_TOKEN:-}" ] && bail "Error: GITHUB_TOKEN environment variable is not set"

        # Make sure we can execute the GitHub command line tool.
        HUB=${HUB:-hub}
        command -v $HUB &>/dev/null || bail "Error: Could not execute GitHub cmdline tool 'hub'"
    fi
fi

# Make sure we have the checksum generator sha256sum.
command -v sha256sum &>/dev/null || bail "Error: Could not execute sha256sum"

# Make sure we are on the main branch.
[ "$(git branch --show-current)" != "main" ] && bail "Error: Not on the main branch"

if [ $check_git_status -eq 1 ] ; then 
    # We only generate releases from a git repository clone that does not have any
    # uncommitted modifications or untracked files.
    [ -n "$(git status --porcelain)" ] && bail "Error: Your git repository clone is not clean"
fi

previous_tag=$(git describe --abbrev=0)
[ -z "$previous_tag" ] && bail "Error: Could not detect the previous release tag"

new_tag="v$version"
[ -z "$(git tag -l $new_tag)" ] || bail "Error: Release tag $new_tag already exists."

name="$basename-$version"

#####################################################
##### Build everything in a temporary directory #####
#####################################################

# Create a temporary directory and clone the current git clone into the
# temporary directory.
tmp_dir="$(mktemp -d)"
trap "rm -rf '$tmp_dir'" EXIT

firmware_dir="$tmp_dir/firmware"
clone_dir="$tmp_dir/clone"
mkdir -p "$firmware_dir" "$clone_dir"

git clone . "$clone_dir"

# Switch to the clone in temporary directory and update its submodules.
cd "$clone_dir"
echo "Updating git submodules ..."
git submodule update --init

# Create a signed and annotated tag in the local git repository clone. Fail if
# the tag already exists.
echo -n "Creating git tag $new_tag ... "
if [ $sign -eq 1 ]; then
    git_opts="-s"
fi
git tag ${git_opts:-} -a "$new_tag" -m "Version $version"
echo "done."

# Copy the resulting binary files into the firmware release directory.
install_firmware()
{
    cp -f out/release/firmware.bin "$firmware_dir/$name.$1.bin"
    cp -f out/debug/firmware.bin   "$firmware_dir/$name.$1.debug.bin"
}

make clean
make debug
make release
install_firmware tower

# Compute SHA-256 checksums of all firmware release files.
cd "$firmware_dir"
firmware_files=(*)
checksums=$(sha256sum -b ${firmware_files[*]})

if [ $sign -eq 1 ] ; then
    # Generate a signed version of the checksums.
    echo -n "Signing the release manifest ... "
    signed_checksums=$(echo "$checksums" | gpg --clear-sign)
    echo "done."
else
    signed_checksums="$checksums"
fi

# Generate a manifest file with SHA-256 checksums of all release files.
cat > $firmware_dir/manifest.md << EOF
Release $version

**SHA256 checksums**:
\`\`\`txt
$signed_checksums
\`\`\`

**Full changelog**: https://github.com/irtlab/$basename/compare/$previous_tag...$new_tag
EOF

if [ $dry_run -eq 1 ] ; then
    echo "Dry run requested, discarding build artifacts."
    exit 0
fi

############################################################
##### Copy tag and binary files back go original clone #####
############################################################

# Push the newly created signed release tag back to the local git clone from
# which we created the clone in the temporary directory.
cd "$clone_dir"
git push origin "$new_tag"

# Copy all files generated in this release back to the original clone.
rm -rf "$orig_dir/release/$version"
mkdir -p "$orig_dir/release/$version"
cp -a "$firmware_dir"/* "$orig_dir/release/$version"

if [ $push_to_github -eq 1 ] ; then
    ##########################
    ##### Push to GitHub #####
    ##########################

    # Push the signed tag that represents the new release to GitHub.
    echo -n "Pushing tag $new_tag to GitHub ... "
    cd "$orig_dir"
    git push origin "$new_tag"
    echo "done."

    cd "release/$version"

    # Create a new GitHub draft release for the new signed release tag with all the
    # generated files attached.
    echo -n "Creating a new GitHub draft release ... "
    attachments=""
    for f in ${firmware_files[@]}; do
        attachments="-a $f $attachments"
    done
    $HUB release create -d $attachments -F manifest.md "$new_tag"
    echo "done."
fi
