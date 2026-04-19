# 🔑 Getting a Yandex.Disk OAuth Token

## 1. Register an application

Create a Yandex OAuth application using the official developer portal.

Official guide:
https://yandex.ru/dev/id/doc/ru/register-client

## 2. Save the client ID

After registration, copy your application's client ID.

## 3. Open the authorization URL

Replace `YOUR_CLIENT_ID` and open this URL in the browser:

```text
https://oauth.yandex.ru/authorize?response_type=token&client_id=YOUR_CLIENT_ID
```

## 4. Approve access

Sign in with your Yandex account and grant the requested permissions.

## 5. Copy the token

After successful authorization, the access token will be returned in the redirect URL fragment.

## 6. Export the token locally

Linux/macOS:

```bash
export YADISK_TOKEN="your_token_here"
```

Windows PowerShell:

```powershell
$env:YADISK_TOKEN="your_token_here"
```

## Notes

- Never commit real tokens to Git
- Do not hardcode tokens in examples
- Prefer environment variables for local development