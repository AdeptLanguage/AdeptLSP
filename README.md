# AdeptLSP
Adept Language Server

## Installation

[Download `adeptls` binary for your platform](https://github.com/AdeptLanguage/AdeptLSP/releases/tag/nightly) or build it from source, then follow the directions below for your editor:

#### Neovim

You can add something like the following to your `init.lua`.

Make sure to replace the path given for `--infrastructure` with the output of running `adept --root`.

Also make sure to replace the path to `adeptls` inside the `cmd` table with the path to it on your system.

```
vim.api.nvim_create_autocmd("FileType", {
    pattern = "adept",
    callback = function()
        local client = vim.lsp.start({
            name = "adeptls",
            cmd = {
                "/Users/isaac/AdeptProjects/AdeptLSP/adeptls",
                "--infrastructure",
                "/Users/isaac/Projects/Adept/build/macOS-Release/"
            },
        })
        vim.lsp.buf_attach_client(0, client)
    end
})
```

#### Sublime Text

- Step 1: Install [LSP Package](https://github.com/sublimelsp/LSP) from Package Control 
- Step 2: Open LSP Settings by clicking menu `Sublime Text -> Settings -> Package Settings -> LSP -> Settings`
- Step 3: Add client for `adeptls` to configuration. Example `LSP.sublime-settings`:
- Step 4: Make sure to replace the path given for `--infrastructure` with the output of running `adept --root`.
- Step 5: Also make sure to replace the path to `adeptls` inside the `command` array with the path to it on your system.

```
{
    "clients": {
        "adeptls": {
            "enabled": true,
            "command": [
                "/Users/isaac/AdeptProjects/AdeptLSP/adeptls",
                "--infrastructure",
                "/Users/isaac/Projects/Adept/build/macOS-Release/"
            ],
            "selector": "source.adept"
        }
    }
}
```

## Compiling from Scratch

You can compile `adeptls` by navigating to the project folder, and then running:

```
make
```

(and then the result is `./adeptls`)

##### Why Makefile?

Since this LSP relies on C code for the actual code insight, a `Makefile` is provided.

The `Makefile` builds both the frontend language server which handles processing/communication and the backend insight server which performs the code analysis.

To only rebuild the frontend, you can build it like a normal Adept project using:

```
adept
```

