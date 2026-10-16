root:

    venv\Scripts\activate

    python -m pip install --upgrade pip

1st terminal (activated via venv):
    python -m pip install awscli "moto[server]"

2nd terminal:
    set AWS_ACCESS_KEY_ID=test
    set AWS_SECRET_ACCESS_KEY=test
    set AWS_DEFAULT_REGION=us-east-1
    set AWS_ENDPOINT_URL=http://127.0.0.1:5001

    checks:
    aws --endpoint-url http://127.0.0.1:5001 sts get-caller-identity
    aws --endpoint-url http://127.0.0.1:5001 s3 mb s3://check-bucket
    aws --endpoint-url http://127.0.0.1:5001 s3 ls
    aws --endpoint-url http://127.0.0.1:5001 ec2 describe-availability-zones `
        --query "AvailabilityZones[].ZoneName" `
        --output table




! Windows:
change in venv/Scripts/aws.cmd:
    from this: "for /f "tokens=2 delims==" %%i in ('assoc .py') do (."
    to this:   "for /f "tokens=2 delims==" %%i in ('assoc .py 2^> nul') do (."
    why: to remove warning "Не найдено сопоставление для расширения имени файла .py."

