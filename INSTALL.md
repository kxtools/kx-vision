# KX-Vision Installation Guide

> **Quick Start:** After installing, the overlay may be hidden by default.
> **Press the `INSERT` key** in-game to show or hide the main window.

KX-Vision supports multiple addon loaders. Please choose the method that best fits your setup.

---

### Method 1: Installation with Raidcore Nexus (Recommended)

This is the recommended method for most users. It allows KX-Vision to run alongside other popular addons within the Nexus ecosystem.

**Step 1: Install Raidcore Nexus**

You have two options to install Nexus. The installer is the easiest.

*   **Option A: Using the Installer (Easiest)**
    1.  Download the **Nexus Installer (.exe)** from the [official releases page](https://github.com/RaidcoreGG/NexusInstaller/releases/latest/download/NexusInstaller.exe).
    2.  Run `NexusInstaller.exe`.
    3.  Click the **"Locate Game"** button and select your `Gw2-64.exe` file (usually found in `C:\Program Files\Guild Wars 2\`).
    4.  Click **"Install"**.
    5.  Launch Guild Wars 2 once. Nexus will perform its initial setup, and you should see its interface appear in-game.

*   **Option B: Manual DLL Installation (Advanced)**
    1.  Download the **Nexus DLL** directly from the [official releases page](https://github.com/RaidcoreGG/Nexus/releases/latest/download/d3d11.dll).
    2.  Place the downloaded `d3d11.dll` file into your main **`Guild Wars 2`** game directory.
    3.  Launch the game once to let Nexus generate its folders.

**Step 2: Install the D3D9 Wrapper**

*This wrapper is still required for compatibility, even when using Nexus.*

> **Tip:** You can easily find your addons folder by opening the Nexus menu in-game (default Ctrl+O), going to the Addons tab, and clicking the **"Open Addons Folder"** button.

1.  Go to the [**d3d9_wrapper releases page**](https://github.com/gw2-addon-loader/d3d9_wrapper/releases/latest).
2.  Download the `d3d9_wrapper_*.zip` file.
3.  Place the extracted **`d3d9_wrapper` folder** from the zip into your `addons` folder.
    -   *(The final path will be `C:\Program Files\Guild Wars 2\addons\d3d9_wrapper\`)*

**Step 3: Install KX-Vision**

1.  Go to the [**KX-Vision Latest Release page**](https://github.com/kxtools/kx-vision/releases/latest).
2.  Download the `kx-vision_gw2al.zip` file.
3.  Create a new folder named `kxvision` inside your `Guild Wars 2/addons/` directory.
    -   *(The final path will be `C:\Program Files\Guild Wars 2\addons\kxvision\`)*
4.  Place the `gw2addon_kxvision.dll` from the zip into this new `kxvision` folder.
5.  Launch the game! Press **`INSERT`** to toggle the overlay.

---

### Method 2: Installation with GW2 Addon Loader (Manual)

Use this method if you prefer a minimal setup without Raidcore Nexus.

**Step 1: Install the GW2 Addon Loader Core**

*If you already have a working addon loader setup (e.g., for GW2Radial), you can likely skip this step.*

1.  Go to the official [**GW2 Addon Loader releases page**](https://github.com/gw2-addon-loader/loader-core/releases/latest).
2.  Download the latest `loader_core_*.zip` file.
3.  Extract the contents of the zip (`bin64` folder, `addonLoader.dll`, `d3d11.dll`, `dxgi.dll`) directly into your main **`Guild Wars 2`** game directory.
    -   *(The final path for the DLLs will be `C:\Program Files\Guild Wars 2\d3d11.dll`, etc.)*

**Step 2: Install the D3D9 Wrapper**

*This wrapper is required for addon compatibility. If you already have it from another addon, you can skip this step.*

1.  Go to the [**d3d9_wrapper releases page**](https://github.com/gw2-addon-loader/d3d9_wrapper/releases/latest).
2.  Download the `d3d9_wrapper_*.zip` file.
3.  Find or create the `addons` folder inside your `Guild Wars 2` directory.
4.  Place the extracted **`d3d9_wrapper` folder** from the zip into your `addons` folder.
    -   *(The final path will be `C:\Program Files\Guild Wars 2\addons\d3d9_wrapper\`)*

**Step 3: Install KX-Vision**

1.  Go to the [**KX-Vision Latest Release page**](https://github.com/kxtools/kx-vision/releases/latest).
2.  Download the `kx-vision_gw2al.zip` file.
3.  Create a new folder named `kxvision` inside your `Guild Wars 2/addons/` directory.
    -   *(The final path will be `C:\Program Files\Guild Wars 2\addons\kxvision\`)*
4.  Place the `gw2addon_kxvision.dll` from the zip into this new `kxvision` folder.
5.  Launch the game! Press **`INSERT`** to toggle the overlay.

---

### Method 3: Manager-Assisted Installation (GW2 Addon Manager)

This method uses the stand-alone GW2 Addon Manager to handle the loader and wrapper dependencies automatically.

1.  Download and run the [**GW2 Addon Manager**](https://github.com/gw2-addon-loader/GW2-Addon-Manager).
2.  Allow the manager to detect your game.
3.  Inside the manager window, click the **"Update"** button. This will install the core loader and required wrappers for you.
4.  Once the update is complete, follow **Step 3** from **Method 2 (Manual Installation)** above to install KX-Vision itself.
5.  Launch the game! Press **`INSERT`** to toggle the overlay.

---

### Method 4: Standalone DLL Injection (Advanced Users / Developers)

This method does not use an addon loader and is intended for development or testing purposes.

1.  Go to the [**KX-Vision Latest Release page**](https://github.com/kxtools/kx-vision/releases/latest).
2.  Download the `kx-vision_standalone.zip` file.
3.  Launch Guild Wars 2.
4.  Inject the `KX-Vision.dll` into the game process using an injector tool.
5.  Press **`INSERT`** to show the overlay and **`DELETE`** to unload.