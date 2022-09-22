# lorabridge development repo

## Developer setup
In order to better separate and organize individual parts of our project, we created a dedidacted repository for each part and combined them via git submodules in the `lorabridge` repository.

### Clone Repository

You can initially set up the repository with:

```bash
git clone --recursive git@github.com:lorabridge/lorabridge.git
```

Git will clone the main repository `lorabridge` along with all submodules.


### Get latest updates

You can pull updates with either:

```bash
git pull --recurse-submodules
```

or

```bash
git submodule update --recursive --remote
```

### Develop a new submodule

To add a new module, just create a new repository, create stuff and then add your repository as a submodule to the main repository `lorabridge`.

You can use following commands:

```bash
git submodule add git@github.com:lorabridge/<repo_name>.git <folder-name>
git config -f .gitmodules submodule.<repo_name>.update merge
```

### Modify something inside a submodule

The intended purpose of `git submodules` is to include a specific snapshot of another repository into your own.
Therefore, after each pull `git submodules` detaches the head from main to the state included in the main repository `lorabridge`.

Drawbacks of this behaviour:

- The checked out submodule (e.g. bridge/bridge-lorawan_tx) is currently not on origin/main
- The checked out submodule (e.g. bridge/bridge-lorawan_tx) is currently not on the latest commit
- Committed changes do not belong to any branch

The easiest workflow in order to do changes directly in a submodules would be to:

- Pull updates with `git pull --recurse-submodules`
- Check out the main branch __before__ you change anything with `cd <path/of/submodule>; git checkout main`
  - > You can checkout the master branch for all submodules at once with `git submodules foreach -q --recursive 'git checkout main'`
- Change stuff
- Add your files (`cd <path/of/submodule>; git add <specific file or . for all>`)
- Commit your changes (`git commit`)
- Push your changes (`git push`) (optional, can be done later)

__Attention: Since `git submodules` always uses _snapshots_, we need to tell the main repository `lorabridge` that it should use the new commit of the submodule.__

- Navigate into the main repository `lorabridge`
- Add the new state of the submodule (`git add <path of submodule>`)
- Commit the new state (`git commit`)
- Push the changes `git push` (or `git push --recurse-submodules=on-demand` to push commit inside submodules as well)

__I forgot to do `git checkout main` _before_ I changed stuff:__  
No worries, nothing is lost. The workflow is just a little more annoying.

- Just finish your changes
- Add your changes files with `git add <file or . for all>`
- Commit your changes with `git commit`
  - > Now your commit exists, but it does not belong to any branch
- Retrieve the commit hash of your commit with `git rev-parse HEAD` or use the top hash shown via `git log`
- Switch to the main branch with `git checkout main`
- Merge your commit with `git merge <hash>`
- Now follow the same procedure as above: Push your changes, switch to the main repository, add and commit the new state of the submodule and push this as well

## Raspberry deployment

- Install Docker and Docker-compose, add `pi` user to `docker` group (`exec su -l $USER`)
- Generate ssh keys and add public key to `lorabrigepi` github account (`Settings` - `SSH and GPG keys` - `New SSH key`)
- Clone lorabridge repositores
```bash
git clone git@github.com:lorabridge/lorabridge.git
```
- Generate personal access token for access to the github packages repository (`Settings`- `Developer Settings` - `Generate new token`- Add scope `read:packages` )
- Copy the token to `~/.github_pac`
- Login to the registry
```bash
cat ~/.github_pac | docker login ghcr.io -u lorabridgepi --password-stdin
```