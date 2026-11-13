# Landing page e dashboard

HTML, CSS e JavaScript sem framework.

| Arquivo | Conteúdo |
|---|---|
| `index.html` | Landing page do projeto |
| `dashboard.html` | Painel ao vivo: estado, sensores, motores e histórico |
| `assets/config.js` | Endereço da API e id do robô |
| `assets/dashboard.js` | Consulta `GET /robots/{id}/latest` a cada 1 s |
| `assets/style.css` | Estilo das duas páginas |

Para testar, rode a API (veja `backend/`), ajuste `API_URL` em `assets/config.js` e abra `index.html` no navegador.

O site pode ser publicado de graça no **GitHub Pages** (Settings → Pages → pasta `/dashboard`) ou na Vercel.
