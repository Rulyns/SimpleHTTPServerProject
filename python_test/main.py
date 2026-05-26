from fastapi import FastAPI
from fastapi.responses import PlainTextResponse

app = FastAPI(docs_url=None, redoc_url=None, openapi_url=None)


@app.get("/", response_class=PlainTextResponse)
async def root() -> str:
    return "Hello, World"
