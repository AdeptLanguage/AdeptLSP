# AdeptLSP
Adept Language Server

## Installation

Download `adeptls` binary or build it from source, then follow the directions below for your editor:

#### Neovim

You can add something like the following to your `init.lua`.

Make sure to replace the path given for `--infrastructure` with the output of running `adept --root`.

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
```
make
```

And then the result is `./adeptls`

Since this LSP relies on C code, a `Makefile` is provided.

It builds both the frontend language server as well as the backend insight server.

If you only want to build the frontend, you can build it like a normal Adept project using:

```
adept
```

