# AdeptLSP
Adept Language Server

### How to build:
```
make
```

And then the result is `./adept-lsp`

Since this LSP relies on C code, a `Makefile` is provided.

It builds both the frontend language server as well as the backend insight server.

If you only want to build the frontend, you can build it like a normal Adept project using:

```
adept
```

