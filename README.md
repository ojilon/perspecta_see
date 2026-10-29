# perspecta_see

See how an html/css/js/c project fits together, and edit it from inside the browser.

    .\build.ps1 -Run        # then open http://127.0.0.1:8787

The C program is a tiny local server: it scans a project folder, parses its files, and
sends the files, functions and links to the browser as JSON.