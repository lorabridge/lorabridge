# lorabridge development repo

## Developer setup
setup with:

```bash
git clone --recursive git@github.com:lorabridge/lorabridge.git
```

pull updates with:

```bash
git pull --recurse-submodules
```

add new subodules with:

```bash
git submodule add git@github.com:lorabridge/<repo_name>.git <folder-name>
```

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