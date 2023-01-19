<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->



<a name="readme-top"></a>


<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="[https://github.com/lorabridge/lorabridge](https://github.com/lorabridge/lorabridge)">
    <img src="https://github.com/lorabridge/docs-internal/blob/main/literature/docs/assets/loradatabridge_logo_large.png" alt="Logo" width="220">
  </a>

  <h2 align="center">Long-Range Data Bridge (LoRaBridge)</h2>

  <p align="center">
    Convert your cheap Zigbee sensors into LoRaWAN sensors!
    <br />
    <a href="https://github.com/othneildrew/Best-README-Template"><strong>Explore the docs »</strong></a>   
  </p>
</div>

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#features">Features</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project


<img src="https://github.com/lorabridge/docs-internal/blob/main/literature/docs/assets/lorabridge_frontpage_system_diagram.png" alt="Logo" width="1000">
  

LoRaBridge delivers range extension for wireless sensors via LoRaWAN. Hence, e.g., home automation sensors can be mounted
in locations, such as cellars, attics or garden houses, which are hard to reach with medium/short range connectivity.

The two key components of LoRaBridge are: 
1) <i> Bridge unit </i> - Collects, compresses and transmits sensor data over LoRaWAN 
2) <i> Gateway unit </i> - Receives, decompresses and forwards the sensor data to a backend system. 

Currently the full installation of LoRaBridge connects Zigbee sensors (supported by <a href="https://www.zigbee2mqtt.io/">Zigbee2MQTT</a>) to <a href="https://www.home-assistant.io/">Home Assistant</a>. 
For details on how to customize LoRaBridge for your own needs, please refer to the developer manual (TODO: add link)


### Features

- Compression and scheduling algorithms enable range extension for multiple Zigbee sensors over a single LoRaWAN connection
- Easy to use single button UI for Zigbee device pairing
- Web UI for device management
- Preinstalled Zigbee2MQTT and Home Assistant containers
- Ansible setup scripts for automated deployment

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
- Add your changed files with `git add <file or . for all>`
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

## Known issues and limitations

- Range extension is available only for uplink communication (e.g. Zigbee sensor values)
- Range extensions over large distances may likely cause data loss due to Zigbee sensor data rate is exceeded by the LoRaWAN link data rate.
- Operation has been verified with a small subset of sensors supported by Zigbee2MQTT
- LoRaWAN reception might become instable due to lack of realtime support in Raspberry PI OS

## License

All the LoRaBridge software components and the documentation are licensed under GNU General Public License 3.0.

## Contact

For business inquiries or research co-operations, please refer to 

## Acknowledgements

The financial support from Internetstiftung/Netidee is gratefully acknowledged. The mission of Netidee is to support development of open-source tools for more accessible and versatile use of the Internet in Austria.





<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/othneildrew/Best-README-Template.svg?style=for-the-badge
[contributors-url]: https://github.com/lorabridge/lorabridge/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/othneildrew/Best-README-Template.svg?style=for-the-badge
[forks-url]: https://github.com/othneildrew/Best-README-Template/network/members
[stars-shield]: https://img.shields.io/github/stars/othneildrew/Best-README-Template.svg?style=for-the-badge
[stars-url]: https://github.com/othneildrew/Best-README-Template/stargazers
[issues-shield]: https://img.shields.io/github/issues/othneildrew/Best-README-Template.svg?style=for-the-badge
[issues-url]: https://github.com/othneildrew/Best-README-Template/issues
[license-shield]: https://img.shields.io/github/license/othneildrew/Best-README-Template.svg?style=for-the-badge
[license-url]: https://github.com/othneildrew/Best-README-Template/blob/master/LICENSE.txt
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/othneildrew
[product-screenshot]: images/screenshot.png
[Next.js]: https://img.shields.io/badge/next.js-000000?style=for-the-badge&logo=nextdotjs&logoColor=white
[Next-url]: https://nextjs.org/
[React.js]: https://img.shields.io/badge/React-20232A?style=for-the-badge&logo=react&logoColor=61DAFB
[React-url]: https://reactjs.org/
[Vue.js]: https://img.shields.io/badge/Vue.js-35495E?style=for-the-badge&logo=vuedotjs&logoColor=4FC08D
[Vue-url]: https://vuejs.org/
[Angular.io]: https://img.shields.io/badge/Angular-DD0031?style=for-the-badge&logo=angular&logoColor=white
[Angular-url]: https://angular.io/
[Svelte.dev]: https://img.shields.io/badge/Svelte-4A4A55?style=for-the-badge&logo=svelte&logoColor=FF3E00
[Svelte-url]: https://svelte.dev/
[Laravel.com]: https://img.shields.io/badge/Laravel-FF2D20?style=for-the-badge&logo=laravel&logoColor=white
[Laravel-url]: https://laravel.com
[Bootstrap.com]: https://img.shields.io/badge/Bootstrap-563D7C?style=for-the-badge&logo=bootstrap&logoColor=white
[Bootstrap-url]: https://getbootstrap.com
[JQuery.com]: https://img.shields.io/badge/jQuery-0769AD?style=for-the-badge&logo=jquery&logoColor=white
[JQuery-url]: https://jquery.com 
