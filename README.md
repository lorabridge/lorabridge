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
  

LoRaBridge delivers range extension for wireless ZigBee sensors via LoRaWAN. Hence, e.g., home automation sensors can be mounted
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
