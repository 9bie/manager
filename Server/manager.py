import app
try:
    app.create_app()
except KeyboardInterrupt:
    print("Bye~")
    app.close_app()

