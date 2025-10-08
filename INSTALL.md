# KX-Vision Installation Guide

> **Quick Start:** After installing, the overlay may be hidden by default.
> **Press the `INSERT` key** in-game to show or hide the main window.

KX-Vision requires the GW2 Addon Loader and its dependencies. Please choose one of the primary methods below to get started.

---

### Method 1: Using the GW2 Addon Manager (Easiest Method)

This is the **easiest and most recommended method for all new users**. The manager automatically handles the installation of the core loader and all required dependencies.

**Step 1: Install Dependencies with the Manager**

1.  Download and run the [**GW2 Addon Manager**](https://github.com/gw2-addon-loader/GW2-Addon-Manager/releases/latest).
2.  Allow the manager to detect your game installation.
3.  Click the **"Update"** button. The manager will automatically download and install the required core components (`loader_core` and `d3d9_wrapper`).

> **Tip:** If you encounter issues, go to the "Options" tab in the manager and click "Force Redownload" before trying the update again.

**Step 2: Install KX-Vision**

1.  Go to the [**KX-Vision Latest Release page**](https://github.com/kxtools/kx-vision/releases/latest).
2.  Download the `kx-vision_gw2al.zip` file.
3.  Find your `Guild Wars 2/addons/` directory. The manager will have created this for you.
4.  Create a new folder named `kxvision` inside the `addons` folder.
    -   *(The final path will be `C:\Program Files\Guild Wars 2\addons\kxvision\`)*
5.  Place the `gw2addon_kxvision.dll` from the zip into this new `kxvision` folder.
6.  Launch the game! Press **`INSERT`** to toggle the overlay.

---

### Method 2: Manual Installation (Without Managers)

Use this method if you prefer to install all components by hand without using a manager.

**Step 1: Install the GW2 Addon Loader Core**

1.  Go to the official [**GW2 Addon Loader releases page**](https://github.com/gw2-addon-loader/loader-core/releases/latest).
2.  Download the latest `loader_core_*.zip` file.
3.  Extract the contents of the zip (`d3d11.dll`, `dxgi.dll`, etc.) directly into your main **`Guild Wars 2`** game directory.
    -   *(The final path for the DLLs will be `C:\Program Files\Guild Wars 2\d3d11.dll`, etc.)*

**Step 2: Install the D3D9 Wrapper**

1.  Go to the [**d3d9_wrapper releases page**](https://github.com/gw2-addon-loader/d3d9_wrapper/releases/latest).
2.  Download the `d3d9_wrapper_*.zip` file.
3.  Find or create the `addons` folder inside your `Guild Wars 2` directory.
4.  Place the extracted **`d3d9_wrapper` folder** from the zip into your `addons` folder.
    -   *(The final path will be `C:\Program Files\Guild Wars 2\addons\d3d9_wrapper\`)*

**Step 3: Install KX-Vision**

1.  Go to the [**KX-Vision Latest Release page**](https://github.com/kxtools/kx-vision/releases/latest).
2.  Download the `kx-vision_gw2al.zip` file.
3.  Create a new folder named `kxvision` inside your `Guild Wars 2/addons/` directory.
4.  Place the `gw2addon_kxvision.dll` from the zip into this new `kxvision` folder.
5.  Launch the game! Press **`INSERT`** to toggle the overlay.

---

### Adding Raidcore Nexus to an Existing Setup (Optional)

If you have already installed the GW2 Addon Loader using either **Method 1** or **Method 2**, you can easily add Raidcore Nexus to run alongside it. The Nexus installer will automatically detect your existing `d3d11.dll` (the addon loader) and configure itself to load it properly.

1.  Download the **Nexus Installer (.exe)** from the [official releases page](https://github.com/RaidcoreGG/NexusInstaller/releases/latest).
2.  Run `NexusInstaller.exe`.
3.  Click the **"Locate Game"** button and select your `Gw2-64.exe` file.
4.  Click **"Install"**.

That's it! When you next launch the game, Nexus will load first, and then it will chainload the GW2 Addon Loader, allowing all your addons (including KX-Vision) to work together.

---

### Method 3: Standalone DLL Injection (Developers)

This method does not use an addon loader and is intended for development or testing purposes.

1.  Go to the [**KX-Vision Latest Release page**](https://github.com/kxtools/kx-vision/releases/latest).
2.  Download the `kx-vision_standalone.zip` file.
3.  Launch Guild Wars 2.
4.  Inject the `KX-Vision.dll` into the game process using an injector tool.
5.  Press **`INSERT`** to show the overlay and **`DELETE`** to unload.